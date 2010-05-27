/*
 * Copyright (C) 2010 Igalia S.L.
 *
 * Contact: Iago Toral Quiroga <itoral@igalia.com>
 *
 * Authors: Juan A. Suarez Romero <jasuarez@igalia.com>
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
 * SECTION:grl-media
 * @short_description: A multimedia data transfer object
 * @see_also: #GrlData, #GrlMediaBox, #GrlMediaVideo, #GrlMediaAudio, #GrlMediaImage
 *
 * This high level class represents a multimedia item. It has methods to
 * set and get properties like author, title, description, and so on.
 */

#include "grl-media.h"
#include <grilo.h>
#include <stdlib.h>

#undef G_LOG_DOMAIN
#define G_LOG_DOMAIN "grl-media"

#define RATING_MAX  5.00
#define SERIAL_STRING_ALLOC 100

static void grl_media_dispose (GObject *object);
static void grl_media_finalize (GObject *object);

G_DEFINE_TYPE (GrlMedia, grl_media, GRL_TYPE_DATA);

static void
grl_media_class_init (GrlMediaClass *klass)
{
  GObjectClass *gobject_class = (GObjectClass *)klass;

  gobject_class->dispose = grl_media_dispose;
  gobject_class->finalize = grl_media_finalize;
}

static void
grl_media_init (GrlMedia *self)
{
}

static void
grl_media_dispose (GObject *object)
{
  G_OBJECT_CLASS (grl_media_parent_class)->dispose (object);
}

static void
grl_media_finalize (GObject *object)
{
  g_debug ("grl_media_finalize (%s)",
	   grl_data_get_string (GRL_DATA (object),
                                GRL_METADATA_KEY_TITLE));
  g_signal_handlers_destroy (object);
  G_OBJECT_CLASS (grl_media_parent_class)->finalize (object);
}

/**
 * grl_media_new:
 *
 * Creates a new data media object.
 *
 * Returns: a newly-allocated data media.
 **/
GrlMedia *
grl_media_new (void)
{
  return g_object_new (GRL_TYPE_MEDIA,
		       NULL);
}

/**
 * grl_media_set_rating:
 * @media: a media
 * @rating: a rating value
 * @max: maximum rating value
 *
 * This method receives a rating and its scale and normalizes it
 */
void
grl_media_set_rating (GrlMedia *media, gfloat rating, gfloat max)
{
  gfloat normalized_value = (rating * RATING_MAX) / max;
  grl_data_set_float (GRL_DATA (media),
		      GRL_METADATA_KEY_RATING,
		      normalized_value);
}

gchar *
grl_media_serialize (GrlMedia *media,
                     gboolean full)
{
  GList *key;
  GList *keylist;
  GRegex *type_regex;
  GString *serial;
  GrlKeyID grlkey;
  GrlPluginRegistry *registry;
  const GValue *value;
  const gchar *id;
  const gchar *source;
  const gchar *type_name;
  gchar *protocol;

  g_return_val_if_fail (GRL_IS_MEDIA (media), NULL);
  g_return_val_if_fail ((id = grl_media_get_id (media)), NULL);
  g_return_val_if_fail ((source = grl_media_get_source (media)), NULL);

  type_name = g_type_name (G_TYPE_FROM_INSTANCE (media));

  /* Convert typename to scheme protocol */
  type_regex = g_regex_new ("GrlMedia(.*)", 0, 0, NULL);
  protocol = g_regex_replace (type_regex, type_name, -1, 0, "grl\\L\\1\\E", 0, NULL);
  g_regex_unref (type_regex);

  /* Build serial string with escaped components */
  serial = g_string_sized_new (SERIAL_STRING_ALLOC);
  g_string_assign (serial, protocol);
  g_string_append (serial, "://");
  g_string_append_uri_escaped (serial, source, NULL, TRUE);
  g_string_append_c (serial, '/');
  g_string_append_uri_escaped (serial, id, NULL, TRUE);

  g_free (protocol);

  /* Include all properties */
  if (full) {
    g_string_append_c (serial, '?');
    registry = grl_plugin_registry_get_instance ();

    keylist = grl_metadata_key_list_new (GRL_METADATA_KEY_TITLE,
                                         GRL_METADATA_KEY_URL,
                                         GRL_METADATA_KEY_ARTIST,
                                         GRL_METADATA_KEY_ALBUM,
                                         GRL_METADATA_KEY_GENRE,
                                         GRL_METADATA_KEY_THUMBNAIL,
                                         GRL_METADATA_KEY_AUTHOR,
                                         GRL_METADATA_KEY_DESCRIPTION,
                                         GRL_METADATA_KEY_LYRICS,
                                         GRL_METADATA_KEY_SITE,
                                         GRL_METADATA_KEY_DATE,
                                         GRL_METADATA_KEY_MIME,
                                         GRL_METADATA_KEY_LAST_PLAYED,
                                         GRL_METADATA_KEY_DURATION,
                                         GRL_METADATA_KEY_CHILDCOUNT,
                                         GRL_METADATA_KEY_WIDTH,
                                         GRL_METADATA_KEY_HEIGHT,
                                         GRL_METADATA_KEY_BITRATE,
                                         GRL_METADATA_KEY_PLAY_COUNT,
                                         GRL_METADATA_KEY_LAST_POSITION,
                                         GRL_METADATA_KEY_FRAMERATE,
                                         GRL_METADATA_KEY_RATING,
                                         NULL);
    for (key = keylist; key; key = g_list_next (key)) {
      grlkey = POINTER_TO_GRLKEYID (key->data);
      value = grl_data_get (GRL_DATA (media), grlkey);
      if (value) {
        g_string_append_printf (serial,
                                "%s=",
                                GRL_METADATA_KEY_GET_NAME (grl_plugin_registry_lookup_metadata_key (registry,
                                                                                                    grlkey)));
        if (G_VALUE_HOLDS_STRING (value)) {
          g_string_append_uri_escaped (serial,
                                       g_value_get_string (value),
                                       NULL,
                                       TRUE);
        } else if (G_VALUE_HOLDS_INT (value)) {
          g_string_append_printf (serial, "%d", g_value_get_int (value));
        } else if (G_VALUE_HOLDS_FLOAT (value)) {
          g_string_append_printf (serial, "%f", g_value_get_float (value));
        }
        g_string_append_c (serial, '&');
      }
    }

    g_list_free (keylist);

    /* Remove trailing ?/& character */
    g_string_erase (serial, serial->len - 1, -1);
  }

  return g_string_free (serial, FALSE);
}

GrlMedia *
grl_media_unserialize (const gchar *serial)
{
  GHashTable *properties;
  GList *key;
  GList *keylist;
  GMatchInfo *match_info;
  GRegex *protocol_regex;
  GRegex *query_regex;
  GRegex *uri_regex;
  GType type_media;
  GrlKeyID grlkey;
  GrlMedia *media;
  GrlPluginRegistry *registry;
  gchar *escaped_id;
  gchar *escaped_source;
  gchar *id;
  gchar *keyname;
  gchar *keyvalue;
  gchar *protocol;
  gchar *query;
  gchar *source;
  gchar *type_name;
  gchar *value;

  g_return_val_if_fail (serial, NULL);

  uri_regex =
    g_regex_new ("^(grl.*):\\/\\/(.+)\\/([^\\?]+)(?:\\?(.*))?",
                 G_REGEX_CASELESS,
                 0,
                 NULL);
  if (!g_regex_match (uri_regex, serial, 0, &match_info)) {
    g_warning ("Wrong serial %s", serial);
    g_regex_unref (uri_regex);
    return NULL;
  }

  /* Build the media */
  protocol = g_match_info_fetch (match_info, 1);
  protocol_regex = g_regex_new ("(grl)(.?)(.*)", G_REGEX_CASELESS, 0, NULL);
  type_name = g_regex_replace (protocol_regex,
                               protocol,
                               -1 ,
                               0,
                               "GrlMedia\\u\\2\\L\\3\\E",
                               0,
                               NULL);
  g_regex_unref (protocol_regex);
  g_free (protocol);

  type_media = g_type_from_name (type_name);
  if (type_media) {
    media = GRL_MEDIA (g_object_new (type_media, NULL));
  } else {
    g_warning ("There is no type %s", type_name);
    g_free (type_name);
    g_match_info_free (match_info);
    return NULL;
  }

  g_free (type_name);

  /* Add source and id */
  escaped_source = g_match_info_fetch (match_info, 2);
  escaped_id = g_match_info_fetch (match_info, 3);

  source = g_uri_unescape_string (escaped_source, NULL);
  id = g_uri_unescape_string (escaped_id, NULL);

  grl_media_set_source (media, source);
  grl_media_set_id (media, id);

  g_free (escaped_source);
  g_free (escaped_id);
  g_free (source);
  g_free (id);

  /* Check if there are more properties */
  query = g_match_info_fetch (match_info, 4);
  g_match_info_free (match_info);
  if (query) {
    /* Put properties in a table */
    query_regex = g_regex_new ("([^=&]+)=([^=&]+)", 0, 0, NULL);
    properties = g_hash_table_new_full (g_str_hash,
                                        g_str_equal,
                                        g_free,
                                        g_free);
    g_regex_match (query_regex, query, 0, &match_info);
    while (g_match_info_matches (match_info)) {
      value = g_match_info_fetch (match_info, 2);
      g_hash_table_insert (properties,
                           g_match_info_fetch (match_info, 1),
                           g_uri_unescape_string (value, NULL));
      g_free (value);
      g_match_info_next (match_info, NULL);
    }
    g_match_info_free (match_info);
    g_free (query);

    /* Add properties to media */
    keylist = grl_metadata_key_list_new (GRL_METADATA_KEY_TITLE,
                                         GRL_METADATA_KEY_URL,
                                         GRL_METADATA_KEY_ARTIST,
                                         GRL_METADATA_KEY_ALBUM,
                                         GRL_METADATA_KEY_GENRE,
                                         GRL_METADATA_KEY_THUMBNAIL,
                                         GRL_METADATA_KEY_AUTHOR,
                                         GRL_METADATA_KEY_DESCRIPTION,
                                         GRL_METADATA_KEY_LYRICS,
                                         GRL_METADATA_KEY_SITE,
                                         GRL_METADATA_KEY_DATE,
                                         GRL_METADATA_KEY_MIME,
                                         GRL_METADATA_KEY_LAST_PLAYED,
                                         GRL_METADATA_KEY_DURATION,
                                         GRL_METADATA_KEY_CHILDCOUNT,
                                         GRL_METADATA_KEY_WIDTH,
                                         GRL_METADATA_KEY_HEIGHT,
                                         GRL_METADATA_KEY_BITRATE,
                                         GRL_METADATA_KEY_PLAY_COUNT,
                                         GRL_METADATA_KEY_LAST_POSITION,
                                         GRL_METADATA_KEY_FRAMERATE,
                                         GRL_METADATA_KEY_RATING,
                                         NULL);
    registry = grl_plugin_registry_get_instance ();
    for (key = keylist; key; key = g_list_next (key)) {
      grlkey = POINTER_TO_GRLKEYID (key->data);
      keyname =
        GRL_METADATA_KEY_GET_NAME (grl_plugin_registry_lookup_metadata_key (registry,
                                                                            grlkey));
      keyvalue = g_hash_table_lookup (properties, keyname);
      if (keyvalue) {
        switch (grlkey) {
        case GRL_METADATA_KEY_DURATION:
        case GRL_METADATA_KEY_CHILDCOUNT:
        case GRL_METADATA_KEY_WIDTH:
        case GRL_METADATA_KEY_HEIGHT:
        case GRL_METADATA_KEY_BITRATE:
        case GRL_METADATA_KEY_PLAY_COUNT:
        case GRL_METADATA_KEY_LAST_POSITION:
          grl_data_set_int (GRL_DATA (media), grlkey, atoi (keyvalue));
          break;
        case GRL_METADATA_KEY_FRAMERATE:
        case GRL_METADATA_KEY_RATING:
          grl_data_set_float (GRL_DATA (media), grlkey, atof (keyvalue));
          break;
        default:
          grl_data_set_string (GRL_DATA (media), grlkey, keyvalue);
          break;
        }
      }
    }
    g_list_free (keylist);
    g_hash_table_unref (properties);
  }

  return media;
}
