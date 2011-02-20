/*
 * Copyright (C) 2011 Igalia S.L.
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
 * SECTION:grl-data-multi
 * @short_description: Low-level class to store multivalued data
 * @see_also: #GrlMedia, #GrlMediaBox, #GrlMediaVideo, #GrlMediaAudio, #GrlMediaImage
 *
 * This class acts as dictionary where keys and their values can be stored. It
 * is suggested to better high level classes, like #GrlMedia, which
 * provides functions to access known properties.
 */

#include "grl-data-multi.h"
#include "grl-log.h"
#include "grl-plugin-registry.h"

struct _GrlDataMultiPrivate {
  GHashTable *extended_data;
};

static void grl_data_multi_finalize (GObject *object);

#define GRL_DATA_MULTI_GET_PRIVATE(o)                                   \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GRL_TYPE_DATA_MULTI, GrlDataMultiPrivate))

static void free_list_values (GrlKeyID key, GList *values, gpointer user_data);

/* ================ GrlDataMulti GObject ================ */

G_DEFINE_TYPE (GrlDataMulti, grl_data_multi, GRL_TYPE_DATA);

static void
grl_data_multi_class_init (GrlDataMultiClass *klass)
{
  GObjectClass *gobject_class = (GObjectClass *)klass;

  gobject_class->finalize = grl_data_multi_finalize;

  g_type_class_add_private (klass, sizeof (GrlDataMultiPrivate));
}

static void
grl_data_multi_init (GrlDataMulti *self)
{
  self->priv = GRL_DATA_MULTI_GET_PRIVATE (self);
  self->priv->extended_data = g_hash_table_new_full (g_direct_hash,
                                                     g_direct_equal,
                                                     NULL,
                                                     NULL);
}

static void
grl_data_multi_finalize (GObject *object)
{
  GrlDataMulti *mdata = GRL_DATA_MULTI (object);

  g_hash_table_foreach (mdata->priv->extended_data,
                        (GHFunc) free_list_values,
                        NULL);
  g_hash_table_unref (mdata->priv->extended_data);

  G_OBJECT_CLASS (grl_data_multi_parent_class)->finalize (object);
}

/* ================ Utitilies ================ */

static void
free_list_values (GrlKeyID key, GList *values, gpointer user_data)
{
  g_list_foreach (values, (GFunc) g_object_unref, NULL);
  g_list_free (values);
}

/* ================ API ================ */

/**
 * grl_data_multi_new:
 *
 * Creates a new multivalued data object.
 *
 * Returns: a new multivalued data object.
 **/
GrlDataMulti *
grl_data_multi_new (void)
{
  return g_object_new (GRL_TYPE_DATA_MULTI,
		       NULL);
}

/**
 * grl_data_multi_add:
 * @mdata: a multivalued data
 * @prop: a set of related properties with their values
 *
 * Adds a new set of values.
 *
 * All keys in prop must be related among them.
 *
 * mdata will take the ownership of prop, so do not modify it.
 **/
void
grl_data_multi_add (GrlDataMulti *mdata,
                    GrlProperty *prop)
{
  GList *key;
  GList *keys;
  GList *values;
  GrlPluginRegistry *registry;
  const GList *related_keys;
  guint l;

  g_return_if_fail (GRL_IS_DATA_MULTI (mdata));
  g_return_if_fail (GRL_IS_PROPERTY (prop));

  keys = grl_data_get_keys (GRL_DATA (prop));
  if (!keys) {
    /* Ignore empty properties */
    GRL_WARNING ("Empty set of values");
    g_object_unref (prop);
    return;
  }

  /* It is assumed that this property only contains values for related properties.
     Check if it must be inserted in data or in extended_data */

  l = grl_data_multi_length (mdata, keys->data);

  if (l > 0) {
    /* Get the representative element of key */
    registry = grl_plugin_registry_get_default ();
    related_keys =
      grl_plugin_registry_lookup_metadata_key_relation (registry,
                                                        keys->data);

    if (!related_keys) {
      GRL_WARNING ("Related keys not found for key: %s",
                   grl_metadata_key_get_name (related_keys->data));
      g_list_free (keys);
      return;
    }

    values = g_hash_table_lookup (mdata->priv->extended_data,
                                  related_keys->data);
    values = g_list_append (values, prop);
    g_hash_table_insert (mdata->priv->extended_data,
                         related_keys->data,
                         values);
  } else {
    /* Insert it as single valued data */
    for (key = keys; key; key = g_list_next (key)) {
      grl_data_set (GRL_DATA (mdata),
                    key->data,
                    grl_data_get (GRL_DATA (prop), key->data));
    }
    g_object_unref (prop);
  }
  g_list_free (keys);
}

/**
 * grl_data_multi_length:
 * @mdata: a multivalued data
 * @key: a metadata key
 *
 * Returns how many values key has in mdata.
 *
 * Returns: number of values
 **/
guint
grl_data_multi_length (GrlDataMulti *mdata,
                       GrlKeyID key)
{
  GrlPluginRegistry *registry;
  const GList *related_keys;
  gboolean found = FALSE;
  guint length;

  g_return_val_if_fail (GRL_IS_DATA_MULTI (mdata), 0);

  /* Get the representative element of key */
  registry = grl_plugin_registry_get_default ();
  related_keys = grl_plugin_registry_lookup_metadata_key_relation (registry,
                                                                   key);

  if (!related_keys) {
    GRL_WARNING ("Related keys not found for key: %s",
                 grl_metadata_key_get_name (related_keys->data));
    return 0;
  }

  /* Check first extended data */
  length = g_list_length (g_hash_table_lookup (mdata->priv->extended_data,
                                               related_keys->data));
  if (length == 0) {
    /* Check if we can find the information in data. It is a success if there is
       at least one value for one of the related keys */
    while (related_keys && !found) {
      found = grl_data_key_is_known (GRL_DATA (mdata), related_keys->data);
      related_keys = g_list_next (related_keys);
    }

    if (found) {
      return 1;
    } else {
      return 0;
    }
  } else {
    return length + 1;
  }
}

/**
 * grl_data_multi_get:
 * @mdata: a multivalued data
 * @key: a metadata key
 * @pos: element to retrieve, starting at 0
 *
 * Returns a set containing the values for key and related keys at position specified.
 *
 * Returns: a new #GrlProperty. It must be freed with #g_object_unref()
 **/
GrlProperty *
grl_data_multi_get (GrlDataMulti *mdata,
                    GrlKeyID key,
                    guint pos)
{
  GrlData *collect_from = NULL;
  GrlPluginRegistry *registry;
  GrlProperty *prop = NULL;
  const GList *related_keys;
  GList *list_el;

  g_return_val_if_fail (GRL_IS_DATA_MULTI (mdata), NULL);

  /* Get the representative element of key */
  registry = grl_plugin_registry_get_default ();
  related_keys = grl_plugin_registry_lookup_metadata_key_relation (registry,
                                                                   key);

  if (!related_keys) {
    GRL_WARNING ("Related keys not found for key: %s",
                 grl_metadata_key_get_name (related_keys->data));
    return NULL;
  }

  if (pos == 0) {
    collect_from = GRL_DATA (mdata);
  } else {
    list_el = g_list_nth (g_hash_table_lookup (mdata->priv->extended_data,
                                               related_keys->data),
                          pos - 1);
    if (list_el) {
      collect_from = GRL_DATA (list_el->data);
    } else {
      GRL_WARNING ("Wrong position %u to get data", pos);
    }
  }

  if (collect_from) {
    prop = grl_property_new ();
    while (related_keys) {
      grl_data_set (GRL_DATA (prop),
                    related_keys->data,
                    grl_data_get (collect_from, related_keys->data));
      related_keys = g_list_next (related_keys);
    }
  }

  return prop;
}

/**
 * grl_data_multi_get_all_single:
 * @mdata: a multivalued data
 * @key: a metadata key
 *
 * Returns all non-NULL values for specified key. This ignores completely the
 * related keys.
 *
 * Returns: (element-type GObject.Value) (transfer container): a #GList with
 * values. Free it with #g_list_free.
 **/
GList *
grl_data_multi_get_all_single (GrlDataMulti *mdata,
                               GrlKeyID key)
{
  GList *list_values;
  GList *result = NULL;
  GValue *value;
  GrlPluginRegistry *registry;
  const GList *related_keys;

  g_return_val_if_fail (GRL_IS_DATA_MULTI (mdata), NULL);

  /* Get the representative element of key */
  registry = grl_plugin_registry_get_default ();
  related_keys = grl_plugin_registry_lookup_metadata_key_relation (registry,
                                                                   key);
  if (!related_keys) {
    GRL_WARNING ("Not found related keys");
    return result;
  }

  /* Get the first value */
  value = (GValue *) grl_data_get (GRL_DATA (mdata), key);
  if (value) {
    result = g_list_prepend (result, value);
  }

  /* Get the remaining list of values */
  list_values = g_hash_table_lookup (mdata->priv->extended_data,
                                     related_keys->data);
  while (list_values) {
    value = (GValue *) grl_data_get (GRL_DATA (list_values->data), key);
    if (value) {
      result = g_list_prepend (result, value);
    }
    list_values = g_list_next (list_values);
  }

  return g_list_reverse (result);
}

/**
 * grl_data_multi_get_all_single_string:
 * @mdata: a multivalued data
 * @key: a metadata key
 *
 * Returns all non-NULL values for specified key of type string. This ignores
 * completely the related keys.
 *
 * Returns: (element-type utf8) (transfer container): a #GList with
 * values. Free it with #g_list_free.
 **/
GList *
grl_data_multi_get_all_single_string (GrlDataMulti *mdata,
                                      GrlKeyID key)
{
  GList *list_strings = NULL;
  GList *list_values;
  GList *value;
  const gchar *string_value;

  /* Verify key is of type string */
  if (GRL_METADATA_KEY_GET_TYPE (key) != G_TYPE_STRING) {
    GRL_WARNING ("Wrong type (not string)");
    return NULL;
  }

  list_values = grl_data_multi_get_all_single (mdata, key);
  for (value = list_values; value; value = g_list_next (value)) {
    string_value = g_value_get_string (value->data);
    if (string_value) {
      list_strings = g_list_prepend (list_strings, (gpointer) string_value);
    }
  }

  g_list_free (list_values);

  return g_list_reverse (list_strings);
}

/**
 * grl_data_multi_remove:
 * @mdata: a multivalued data
 * @key: a metadata key
 * @pos: position of key to be removed, starting at 0
 *
 * Removes key and related keys from position in mdata.
 **/
void
grl_data_multi_remove (GrlDataMulti *mdata,
                       GrlKeyID key,
                       guint pos)
{
  GList *list_values;
  GList *to_remove;
  GrlPluginRegistry *registry;
  GrlProperty *prop_at_1;
  const GList *rel_key;
  const GList *related_keys;

  g_return_if_fail (GRL_IS_DATA_MULTI (mdata));

  /* Get the representative element of key */
  registry = grl_plugin_registry_get_default ();
  related_keys = grl_plugin_registry_lookup_metadata_key_relation (registry,
                                                                   key);

  if (!related_keys) {
    GRL_WARNING ("Related keys not found for key: %s",
                 grl_metadata_key_get_name (related_keys->data));
    return;
  }

  /* If element to remove is in position 0 (single-data), then we can replace it
     by value in pos 1 (extended-data) and then remove 1. If element to remove
     is >0 then we must remove it and shift values the empty position */

  if (pos == 0) {
    prop_at_1 = grl_data_multi_get (mdata, key, 1);
    if (prop_at_1) {
      /* Replace related keys in pos 0 */
      for (rel_key = related_keys; rel_key; rel_key = g_list_next (rel_key)) {
        grl_data_set (GRL_DATA (mdata),
                      rel_key->data,
                      grl_data_get (GRL_DATA (prop_at_1), rel_key->data));
      }
      pos = 1;
    } else {
      /* There is no multivalues; remove data */
      for (rel_key = related_keys; rel_key; rel_key = g_list_next (rel_key)) {
        grl_data_remove (GRL_DATA (mdata), rel_key->data);
      }
    }
  }

  if (pos > 0) {
    list_values = g_hash_table_lookup (mdata->priv->extended_data,
                                       related_keys->data);
    to_remove = g_list_nth (list_values, pos - 1);
    if (!to_remove) {
      /* Wrong index; ignore */
      return;
    }

    g_object_unref (to_remove->data);
    list_values = g_list_delete_link (list_values, to_remove);
    g_hash_table_insert (mdata->priv->extended_data,
                         related_keys->data,
                         list_values);
  }
}

/**
 * grl_data_multi_update:
 * @mdata: a multivalued data
 * @prop: a set of related keys
 * @pos: position to be updated
 *
 * Updates the values at position in mdata with values in prop.
 *
 * GrlDataMulti will take ownership of prop, so do not free it after invoking
 * this function.
 **/
void
grl_data_multi_update (GrlDataMulti *mdata,
                       GrlProperty *prop,
                       guint pos)
{
  GList *keys;
  GList *list_val;
  GrlPluginRegistry *registry;
  const GList *related_keys;

  g_return_if_fail (GRL_IS_DATA_MULTI (mdata));
  g_return_if_fail (GRL_IS_PROPERTY (prop));

  keys = grl_data_get_keys (GRL_DATA (prop));
  if (!keys) {
    GRL_WARNING ("Empty properties");
    g_object_unref (prop);
    return;
  }

  /* Get the representative element of key */
  registry = grl_plugin_registry_get_default ();
  related_keys = grl_plugin_registry_lookup_metadata_key_relation (registry,
                                                                   keys->data);

  if (!related_keys) {
    GRL_WARNING ("Related keys not found for key: %s",
                 grl_metadata_key_get_name (related_keys->data));
    return;
  }

  if (pos == 0) {
    while (related_keys) {
      grl_data_set (GRL_DATA (mdata),
                    related_keys->data,
                    grl_data_get (GRL_DATA (prop), related_keys->data));
      related_keys = g_list_next (related_keys);
    }
    g_object_unref (prop);
  } else {
    /* Replace the entire element */
    list_val = g_list_nth (g_hash_table_lookup (mdata->priv->extended_data,
                                                related_keys->data),
                           pos - 1);
    if (!list_val) {
      GRL_WARNING ("%s: index %u out of range", __FUNCTION__, pos);
      g_object_unref (prop);
      return;
    }

    g_object_unref (list_val->data);
    list_val->data = prop;
  }
}
