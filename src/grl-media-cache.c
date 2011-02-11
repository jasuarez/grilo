/*
 * Copyright (C) 2011 Igalia S.L.
 *
 * Contact: Iago Toral Quiroga <itoral@igalia.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

/**
 * SECTION:grl-media_cache
 * @short_description: GrlMedia Cache provider
 * @see_also: #GrlMedia
 *
 * GrlMediaCache subsystem provides a caching backstore to both plugins and
 * clients, where they can cache the #GrlMedia and then recover them for future
 * use.
 */

#include <sqlite3.h>
#include <string.h>

#include "grl-error.h"
#include "grl-log.h"
#include "grl-media-cache.h"

#define GRL_LOG_DOMAIN_DEFAULT media_cache_log_domain
GRL_LOG_DOMAIN(media_cache_log_domain);

#define GRL_MEDIA_CACHE_GET_PRIVATE(object)                   \
  (G_TYPE_INSTANCE_GET_PRIVATE((object),                      \
                               GRL_TYPE_MEDIA_CACHE,          \
                               GrlMediaCachePrivate))

#define GRL_CACHE_DB ".grl-cache"

#define GRL_CACHE_PATTERN "cache_%u"

#define GRL_CACHE_CREATE_CACHE                     \
  "CREATE TABLE %s ("                              \
  "id      TEXT PRIMARY KEY, "                     \
  "parent  TEXT REFERENCES %s (id), "              \
  "updated DATE, "                                 \
  "media   TEXT)"

#define GRL_CACHE_REMOVE_CACHE                  \
  "DROP TABLE %s"

#define GRL_CACHE_INSERT_ELEMENT                \
  "INSERT OR REPLACE INTO %s "                  \
  "(id, parent, updated, media) VALUES "        \
  "(?, ?, ?, ?)"

#define GRL_CACHE_GET_ELEMENT                   \
  "SELECT parent, updated, media from %s "      \
  "WHERE id='%s' "

enum {
  PROP_0,
  PROP_CACHE_ID,
  PROP_PERSISTENT,
};

struct _GrlMediaCachePrivate {
  gchar *cache_id;
  gboolean persistent;
  sqlite3 *db;
};

static void grl_media_cache_finalize (GObject *object);

static void grl_media_cache_dispose (GObject *object);

static void grl_media_cache_get_property (GObject *object,
                                          guint prop_id,
                                          GValue *value,
                                          GParamSpec *pspec);

static void grl_media_cache_set_property (GObject *object,
                                          guint prop_id,
                                          const GValue *value,
                                          GParamSpec *pspec);

static void remove_table (sqlite3 *db,
                          const gchar *name);

/* ================ GrlMediaCache GObject ================ */

static void
grl_media_cache_class_init (GrlMediaCacheClass *cache_class)
{
  GObjectClass *gobject_class;

  gobject_class = G_OBJECT_CLASS (cache_class);

  gobject_class->finalize = grl_media_cache_finalize;
  gobject_class->dispose  = grl_media_cache_dispose;

  gobject_class->set_property = grl_media_cache_set_property;
  gobject_class->get_property = grl_media_cache_get_property;

  g_object_class_install_property (gobject_class,
                                   PROP_CACHE_ID,
                                   g_param_spec_string ("cache-id",
                                                        "Cache identifier",
                                                        "Cache identifier",
                                                        NULL,
                                                        G_PARAM_CONSTRUCT_ONLY |
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class,
                                   PROP_PERSISTENT,
                                   g_param_spec_boolean ("persistent",
                                                         "persistent",
                                                         "Cache is persistent",
                                                         FALSE,
                                                         G_PARAM_READWRITE |
                                                         G_PARAM_STATIC_STRINGS));

  g_type_class_add_private (cache_class,
                            sizeof (GrlMediaCachePrivate));
}

G_DEFINE_TYPE (GrlMediaCache, grl_media_cache, G_TYPE_OBJECT);

static void
grl_media_cache_init (GrlMediaCache *cache)
{
  cache->priv = GRL_MEDIA_CACHE_GET_PRIVATE (cache);
}

static void
grl_media_cache_dispose (GObject *object)
{
  /* Unref all gobject elements */
  /* Careful: it can be invoked several times */
}

static void
grl_media_cache_finalize (GObject *object)
{
  GrlMediaCache *cache = GRL_MEDIA_CACHE (object);

  /* Free all non-gobject elements */
  GRL_DEBUG (__FUNCTION__);
  if (!cache->priv->persistent) {
    remove_table (cache->priv->db, cache->priv->cache_id);
  }
  sqlite3_close (cache->priv->db);
  g_free (cache->priv->cache_id);

  G_OBJECT_CLASS (grl_media_cache_parent_class)->finalize (object);
}

static void
grl_media_cache_set_property (GObject *object,
                              guint prop_id,
                              const GValue *value,
                              GParamSpec *pspec)
{
  GrlMediaCache *cache = GRL_MEDIA_CACHE (object);

  GRL_DEBUG (__FUNCTION__);
  switch (prop_id) {
  case PROP_CACHE_ID:
    cache->priv->cache_id = g_value_dup_string (value);
    break;
  case PROP_PERSISTENT:
    cache->priv->persistent = g_value_get_boolean (value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (cache, prop_id, pspec);
    break;
  }
}

static void
grl_media_cache_get_property (GObject *object,
                              guint prop_id,
                              GValue *value,
                              GParamSpec *pspec)
{
  GrlMediaCache *cache = GRL_MEDIA_CACHE (object);

  GRL_DEBUG (__FUNCTION__);
  switch (prop_id) {
  case PROP_CACHE_ID:
    g_value_set_string (value, cache->priv->cache_id);
    break;
  case PROP_PERSISTENT:
    g_value_set_boolean (value, cache->priv->persistent);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (cache, prop_id, pspec);
    break;
  }
}

/* ================ Utilities ================ */

static sqlite3 *
create_table (const gchar *name)
{
  const gchar *home;
  gchar *db_path;
  gchar *sql_error = NULL;
  gchar *sql_sentence;
  sqlite3 *db;

  home = g_getenv ("HOME");
  if (!home) {
    GRL_WARNING ("$HOME not set, cannot open database");
    return NULL;
  }

  GRL_DEBUG ("Opening database connection...");
  db_path = g_strconcat (home, G_DIR_SEPARATOR_S, GRL_CACHE_DB, NULL);
  if (sqlite3_open (db_path, &db) != SQLITE_OK) {
    g_critical ("Failed ot open database '%s': %s",
                db_path,
                sqlite3_errmsg (db));
    g_free (db_path);
    sqlite3_close (db);
    return NULL;
  }

  g_free (db_path);

  /* Create the table */
  sql_sentence = g_strdup_printf (GRL_CACHE_CREATE_CACHE,
                                  name, name);
  if (sqlite3_exec (db, sql_sentence, NULL, NULL, &sql_error) != SQLITE_OK) {
    if (sql_error) {
      GRL_WARNING ("Failed to create cache '%s': %s", name, sql_error);
      sqlite3_free (sql_error);
    } else {
      GRL_WARNING ("Failed to create cache '%s'", name);
    }
    g_free (sql_sentence);
    sqlite3_close (db);
    return NULL;
  }

  return db;
}

static void
remove_table (sqlite3 *db,
              const gchar *name)
{
  gchar *sql_error = NULL;
  gchar *sql_sentence;

  sql_sentence = g_strdup_printf (GRL_CACHE_REMOVE_CACHE, name);
  if (sqlite3_exec (db, sql_sentence, NULL, NULL, &sql_error) != SQLITE_OK) {
    if (sql_error) {
      GRL_WARNING ("Failed to remove cache '%s': %s", name, sql_error);
      sqlite3_free (sql_error);
    } else {
      GRL_WARNING ("Failed to remove cache '%s'", name);
    }
  }

  g_free (sql_sentence);
}

/* ================ API ================ */


GrlMediaCache *
grl_media_cache_new (void)
{
  GrlMediaCache *cache = NULL;
  gchar *cache_id;
  sqlite3 *db;

  GRL_DEBUG (__FUNCTION__);

  /* Get a name */
  cache_id = g_strdup_printf (GRL_CACHE_PATTERN, g_random_int ());

  /* Create the cache */
  db = create_table (cache_id);

  if (db) {
    cache = g_object_new (GRL_TYPE_MEDIA_CACHE,
                          "cache-id", cache_id,
                          NULL);
    cache->priv->db = db;
  }

  g_free (cache_id);

  return cache;
}

GrlMediaCache *
grl_media_cache_new_persistent (const gchar *cache_id)
{
  GrlMediaCache *cache = NULL;
  sqlite3 *db;

  g_return_val_if_fail (cache_id, NULL);

  GRL_DEBUG (__FUNCTION__);

  /* Create the cache */
  db = create_table (cache_id);

  if (db) {
    cache = g_object_new (GRL_TYPE_MEDIA_CACHE,
                          "cache-id", cache_id,
                          "persistent", TRUE,
                          NULL);
    cache->priv->db = db;
  }

  return cache;
}

gboolean
grl_media_cache_insert_media (GrlMediaCache *cache,
                              GrlMedia *media,
                              const gchar *parent,
                              GError **error)
{
  GTimeVal now;
  gchar *now_str;
  gchar *serial_media;
  gchar *sql_sentence;
  gint r;
  sqlite3_stmt *sql_stmt;

  g_return_val_if_fail (GRL_IS_MEDIA_CACHE (cache), FALSE);
  g_return_val_if_fail (GRL_IS_MEDIA (media), FALSE);

  GRL_DEBUG (__FUNCTION__);

  /* Prepare the sentence */
  sql_sentence = g_strdup_printf (GRL_CACHE_INSERT_ELEMENT,
                                  cache->priv->cache_id);

  if (sqlite3_prepare_v2 (cache->priv->db,
                          sql_sentence,
                          strlen (sql_sentence),
                          &sql_stmt, NULL) != SQLITE_OK) {
    g_free (sql_sentence);
    goto error;
  }
  g_free (sql_sentence);

  /* Insert the values */
  serial_media = grl_media_serialize_extended (media, GRL_MEDIA_SERIALIZE_FULL);
  g_get_current_time (&now);
  now_str = g_time_val_to_iso8601 (&now);

  sqlite3_bind_text (sql_stmt, 1, grl_media_get_id (media), -1, SQLITE_STATIC);
  sqlite3_bind_text (sql_stmt, 2, parent, -1, SQLITE_STATIC);
  sqlite3_bind_text (sql_stmt, 3, now_str, -1, SQLITE_TRANSIENT);
  sqlite3_bind_text (sql_stmt, 4, serial_media, -1, SQLITE_TRANSIENT);

  g_free (serial_media);
  g_free (now_str);

  /* Wait until it finish */
  while ((r = sqlite3_step (sql_stmt)) == SQLITE_BUSY);

  if (r != SQLITE_DONE) {
    sqlite3_finalize (sql_stmt);
    goto error;
  }

  sqlite3_finalize (sql_stmt);

  return TRUE;

 error:
  GRL_WARNING ("Failed to cache media in '%s': %s",
               cache->priv->cache_id,
               sqlite3_errmsg (cache->priv->db));
  g_set_error (error,
               GRL_CORE_ERROR,
               GRL_CORE_ERROR_CACHE_FAILED,
               "Failed to cache media in '%s'",
               cache->priv->cache_id);
  return FALSE;
}

GrlMedia *
grl_media_cache_get_media (GrlMediaCache *cache,
                           const gchar *media_id,
                           gchar **parent,
                           GTimeVal *last_time_changed,
                           GError **error)
{
  GrlMedia *media;
  gchar *sql_sentence;
  gint r;
  sqlite3_stmt *sql_stmt;

  g_return_val_if_fail (GRL_IS_MEDIA_CACHE (cache), NULL);
  g_return_val_if_fail (media_id, NULL);

  GRL_DEBUG (__FUNCTION__);

  sql_sentence = g_strdup_printf (GRL_CACHE_GET_ELEMENT,
                                  cache->priv->cache_id,
                                  media_id);

  if (sqlite3_prepare_v2 (cache->priv->db,
                          sql_sentence,
                          strlen (sql_sentence),
                          &sql_stmt,
                          NULL) != SQLITE_OK) {
    g_free (sql_sentence);
    goto error;
  };

  g_free (sql_sentence);

  while ((r = sqlite3_step (sql_stmt)) == SQLITE_BUSY);

  if (r != SQLITE_ROW) {
    sqlite3_finalize (sql_stmt);
    goto error;
  }

  if (parent) {
    *parent = g_strdup ((const gchar *) sqlite3_column_text (sql_stmt, 0));
  }

  if (last_time_changed) {
    g_time_val_from_iso8601 ((const gchar *) sqlite3_column_text (sql_stmt, 1),
                             last_time_changed);
  }

  media =
    grl_media_unserialize ((const gchar *) sqlite3_column_text (sql_stmt, 2));

  sqlite3_finalize (sql_stmt);

  return media;

 error:
  GRL_WARNING ("Failed to get media '%s' from '%s': %s",
               media_id,
               cache->priv->cache_id,
               sqlite3_errmsg (cache->priv->db));
  g_set_error (error,
               GRL_CORE_ERROR,
               GRL_CORE_ERROR_CACHE_FAILED,
               "Failed to get media '%s' from '%s'",
               media_id,
               cache->priv->cache_id);
  return FALSE;
}
