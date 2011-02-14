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
#include "grl-plugin-registry.h"

#define GRL_LOG_DOMAIN_DEFAULT media_cache_log_domain
GRL_LOG_DOMAIN(media_cache_log_domain);

#define GRL_MEDIA_CACHE_GET_PRIVATE(object)                   \
  (G_TYPE_INSTANCE_GET_PRIVATE((object),                      \
                               GRL_TYPE_MEDIA_CACHE,          \
                               GrlMediaCachePrivate))

#define GRL_CACHE_DB ".grl-cache"

#define GRL_CACHE_PATTERN "cache_%u"

#define GRL_CACHE_CREATE_CACHE_BEGIN               \
  "CREATE %s TABLE %s ("                           \
  "id      TEXT PRIMARY KEY, "                     \
  "parent  TEXT REFERENCES %s (id), "              \
  "updated DATE, "                                 \
  "media   TEXT"

#define GRL_CACHE_CREATE_CACHE_FIELD            \
  ", %s %s"

#define GRL_CACHE_CREATE_CACHE_END              \
  ")"

#define GRL_CACHE_REMOVE_CACHE                  \
  "DROP TABLE %s"

#define GRL_CACHE_CHECK_CACHE                   \
  "SELECT name FROM sqlite_master "             \
  "WHERE type='table' AND name='%s'"

#define GRL_CACHE_DESC_CACHE                    \
  "PRAGMA table_info(%s)"

#define GRL_CACHE_INSERT_ELEMENT                \
  "INSERT OR REPLACE INTO %s "                  \
  "(id, parent, updated, media%s) VALUES "      \
  "(?, ?, ?, ?%s)"

#define GRL_CACHE_GET_ELEMENT                   \
  "SELECT parent, updated, media from %s "      \
  "WHERE id='%s' "

#define GRL_CACHE_SEARCH                        \
  "SELECT cache.media from %s cache "           \
  "%s %s"

enum {
  PROP_0,
  PROP_CACHE_ID,
  PROP_PERSISTENT,
};

struct _GrlMediaCachePrivate {
  gchar *cache_id;
  GList *extra_keys;
  gboolean persistent;
  gboolean force_db_removal;
  sqlite3 *db;
};

static void grl_media_cache_finalize (GObject *object);

static void grl_media_cache_dispose (GObject *object);

static void grl_media_cache_get_property (GObject *object,
                                          guint prop_id,
                                          GValue *value,
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

  gobject_class->get_property = grl_media_cache_get_property;

  g_object_class_install_property (gobject_class,
                                   PROP_CACHE_ID,
                                   g_param_spec_string ("cache-id",
                                                        "Cache identifier",
                                                        "Cache identifier",
                                                        NULL,
                                                        G_PARAM_READABLE |
                                                        G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class,
                                   PROP_PERSISTENT,
                                   g_param_spec_boolean ("persistent",
                                                         "persistent",
                                                         "Cache is persistent",
                                                         FALSE,
                                                         G_PARAM_READABLE |
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

  if (cache->priv->force_db_removal) {
    remove_table (cache->priv->db, cache->priv->cache_id);
  }

  sqlite3_close (cache->priv->db);
  g_free (cache->priv->cache_id);
  g_list_free (cache->priv->extra_keys);

  G_OBJECT_CLASS (grl_media_cache_parent_class)->finalize (object);
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
create_connection ()
{
  const gchar *home;
  gchar *db_path;
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

  return db;
}

static sqlite3 *
create_table (const gchar *name, GList *keys, GList **filtered_keys, gboolean persistent)
{
  GString *sql_build;
  gchar *sql_begin;
  gchar *sql_error = NULL;
  gchar *sql_sentence;
  gchar *sql_type;
  sqlite3 *db;

  g_return_val_if_fail (filtered_keys, NULL);

  db = create_connection ();
  if (!db) {
    return NULL;
  }

  /* Create the table */
  sql_begin = g_strdup_printf (GRL_CACHE_CREATE_CACHE_BEGIN,
                               persistent? "": "TEMPORARY",
                               name, name);

  if (keys) {
    sql_build = g_string_new (sql_begin);
    g_free (sql_begin);
    while (keys) {
      switch (GRL_METADATA_KEY_GET_TYPE (keys->data)) {
      case G_TYPE_INT:
        sql_type = "INT";
        break;
      case G_TYPE_STRING:
        sql_type = "TEXT";
        break;
      case G_TYPE_FLOAT:
        sql_type = "REAL";
        break;
      default:
        sql_type = NULL;
        break;
      }
      if (sql_type) {
        *filtered_keys = g_list_prepend (*filtered_keys, keys->data);
        g_string_append_printf (sql_build,
                                GRL_CACHE_CREATE_CACHE_FIELD,
                                grl_metadata_key_get_name (keys->data),
                                sql_type);
      }
      keys = keys->next;
    }
    g_string_append (sql_build, GRL_CACHE_CREATE_CACHE_END);
    sql_sentence = g_string_free (sql_build, FALSE);
  } else {
    sql_sentence = g_strconcat (sql_begin, GRL_CACHE_CREATE_CACHE_END, NULL);
  }

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

static sqlite3 *
check_table (const gchar *name)
{
  gchar *sql_sentence;
  gint r;
  sqlite3 *db;
  sqlite3_stmt *sql_stmt;

  db = create_connection ();
  if (!db) {
    return NULL;
  }

  /* Prepare the sentence */
  sql_sentence = g_strdup_printf (GRL_CACHE_CHECK_CACHE,
                                  name);

  if (sqlite3_prepare_v2 (db,
                          sql_sentence,
                          strlen (sql_sentence),
                          &sql_stmt, NULL) != SQLITE_OK) {
    g_free (sql_sentence);
    sqlite3_close (db);
    return NULL;
  }

  /* Wait until it finishes */
  while ((r = sqlite3_step (sql_stmt)) == SQLITE_BUSY);

  sqlite3_finalize (sql_stmt);

  /* Check for result */
  if (r == SQLITE_ROW) {
    return db;
  } else {
    sqlite3_close (db);
    return NULL;
  }
}

static GList *
get_table_extra_keys (sqlite3 *db, const gchar *name)
{
  GList *extra_keys = NULL;
  GrlKeyID key;
  GrlPluginRegistry *registry;
  gchar *key_name;
  gchar *sql_sentence;
  gint r, i;
  sqlite3_stmt *sql_stmt;

  sql_sentence = g_strdup_printf (GRL_CACHE_DESC_CACHE, name);

  if (sqlite3_prepare_v2 (db,
                          sql_sentence,
                          strlen (sql_sentence),
                          &sql_stmt, NULL) != SQLITE_OK) {
    g_free (sql_sentence);
    return NULL;
  }

  g_free (sql_sentence);

  /* Wait until it finishes */
  while ((r = sqlite3_step (sql_stmt)) == SQLITE_BUSY);

  /* Skip the first 4 fields; already known */
  while (r == SQLITE_ROW && i < 4) {
    r = sqlite3_step (sql_stmt);
    i++;
  }

  registry = grl_plugin_registry_get_default ();

  while (r == SQLITE_ROW) {
    /* Get field */
    key_name = (gchar *) sqlite3_column_text (sql_stmt, 1);
    key = grl_plugin_registry_lookup_metadata_key (registry, key_name);
    if (key) {
      extra_keys = g_list_prepend (extra_keys, key);
    }
    r = sqlite3_step (sql_stmt);
  }

  sqlite3_finalize (sql_stmt);

  return extra_keys;
}

/* ================ API ================ */


GrlMediaCache *
grl_media_cache_new (GList *keys)
{
  GList *filtered_keys = NULL;
  GrlMediaCache *cache = NULL;
  gchar *cache_id;
  sqlite3 *db;

  GRL_DEBUG (__FUNCTION__);

  /* Get a name */
  cache_id = g_strdup_printf (GRL_CACHE_PATTERN, g_random_int ());

  /* Create the cache */
  db = create_table (cache_id, keys, &filtered_keys, FALSE);

  if (db) {
    cache = g_object_new (GRL_TYPE_MEDIA_CACHE, NULL);
    cache->priv->cache_id = cache_id;
    cache->priv->extra_keys = filtered_keys;
    cache->priv->db = db;
  } else {
    g_free (cache_id);
  }

  return cache;
}

GrlMediaCache *
grl_media_cache_new_persistent (const gchar *cache_id, GList *keys)
{
  GList *filtered_keys = NULL;
  GrlMediaCache *cache = NULL;
  sqlite3 *db;

  g_return_val_if_fail (cache_id, NULL);

  GRL_DEBUG (__FUNCTION__);

  /* Create the cache */
  db = create_table (cache_id, keys, &filtered_keys, TRUE);

  if (db) {
    cache = g_object_new (GRL_TYPE_MEDIA_CACHE, NULL);
    cache->priv->cache_id = g_strdup (cache_id);
    cache->priv->extra_keys = filtered_keys;
    cache->priv->persistent = TRUE;
    cache->priv->db = db;
  }

  return cache;
}

GrlMediaCache *
grl_media_cache_load_persistent (const gchar *cache_id)
{
  GrlMediaCache *cache = NULL;
  sqlite3 *db;

  g_return_val_if_fail (cache_id, NULL);

  GRL_DEBUG (__FUNCTION__);

  db = check_table (cache_id);

  if (db) {
    cache = g_object_new (GRL_TYPE_MEDIA_CACHE, NULL);
    cache->priv->cache_id = g_strdup (cache_id);
    cache->priv->extra_keys = get_table_extra_keys (db, cache_id);
    cache->priv->persistent = TRUE;
    cache->priv->db = db;
  }

  return cache;
}

void
grl_media_cache_destroy (GrlMediaCache *cache)
{
  g_return_if_fail (GRL_IS_MEDIA_CACHE (cache));

  if (cache->priv->persistent) {
    cache->priv->force_db_removal = TRUE;
  }

  g_object_unref (cache);
}

gboolean
grl_media_cache_insert_media (GrlMediaCache *cache,
                              GrlMedia *media,
                              const gchar *parent,
                              GError **error)
{
  GList *key;
  GString *extra_header_str;
  GString *extra_value_str;
  GTimeVal now;
  gchar *extra_header;
  gchar *extra_value;
  gchar *now_str;
  gchar *serial_media;
  gchar *sql_sentence;
  gint r, i;
  sqlite3_stmt *sql_stmt;

  g_return_val_if_fail (GRL_IS_MEDIA_CACHE (cache), FALSE);
  g_return_val_if_fail (GRL_IS_MEDIA (media), FALSE);

  GRL_DEBUG (__FUNCTION__);

  /* Prepare the sentence */
  extra_header_str = g_string_new ("");
  extra_value_str = g_string_new ("");
  key = cache->priv->extra_keys;
  while (key) {
    g_string_append_printf (extra_header_str,
                            ", %s",
                            grl_metadata_key_get_name (key->data));
    g_string_append (extra_value_str, ", ?");
    key = key->next;
  }
  extra_header = g_string_free (extra_header_str, FALSE);
  extra_value = g_string_free (extra_value_str, FALSE);

  sql_sentence = g_strdup_printf (GRL_CACHE_INSERT_ELEMENT,
                                  cache->priv->cache_id,
                                  extra_header,
                                  extra_value);

  g_free (extra_header);
  g_free (extra_value);

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

  i = 5;
  key = cache->priv->extra_keys;
  while (key) {
    switch (GRL_METADATA_KEY_GET_TYPE (key->data)) {
    case G_TYPE_INT:
      sqlite3_bind_int (sql_stmt,
                        i,
                        grl_data_get_int (GRL_DATA (media), key->data));
      break;
    case G_TYPE_FLOAT:
      sqlite3_bind_double (sql_stmt,
                           i,
                           (double) grl_data_get_float (GRL_DATA (media), key->data));
      break;
    default:
      sqlite3_bind_text (sql_stmt,
                         i,
                         grl_data_get_string (GRL_DATA (media), key->data),
                         -1, SQLITE_STATIC);
      break;
    }
    key = key->next;
    i++;
  }

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

GList *
grl_media_cache_search (GrlMediaCache *cache,
                        const gchar *condition,
                        GError **error)
{
  GList *medias = NULL;
  gchar *sql_sentence;
  gint r;
  sqlite3_stmt *sql_stmt;

  g_return_val_if_fail (GRL_IS_MEDIA_CACHE (cache), NULL);

  sql_sentence = g_strdup_printf (GRL_CACHE_SEARCH,
                                  cache->priv->cache_id,
                                  condition? "WHERE": "",
                                  condition? condition: "");

  if (sqlite3_prepare_v2 (cache->priv->db,
                          sql_sentence,
                          strlen (sql_sentence),
                          &sql_stmt,
                          NULL) != SQLITE_OK) {
    g_free (sql_sentence);
    goto error;
  }

  g_free (sql_sentence);

  /* Wait until it finish */
  while ((r = sqlite3_step (sql_stmt)) == SQLITE_BUSY);

  if (r == SQLITE_ERROR) {
    sqlite3_finalize (sql_stmt);
    goto error;
  }

  while (r == SQLITE_ROW) {
    medias =
      g_list_prepend (medias,
                      grl_media_unserialize ((const gchar *) sqlite3_column_text (sql_stmt, 0)));
    r = sqlite3_step (sql_stmt);
  }

  sqlite3_finalize (sql_stmt);

  return medias;

 error:
  GRL_WARNING ("Failed to search in cache '%s': %s",
               cache->priv->cache_id,
               sqlite3_errmsg (cache->priv->db));
  g_set_error (error,
               GRL_CORE_ERROR,
               GRL_CORE_ERROR_CACHE_FAILED,
               "Unable to query cache '%s'",
               cache->priv->cache_id);
  return NULL;
}
