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
 * SECTION:grl-property
 * @short_description: A class where to store related metadata keys.
 * @see_also: #GrlPluginRegistry, #GrlData
 *
 * When handling media keys, like artist, URI, mime-type, and so on, some of
 * these keys are somewhat related: they do not make sense if they are not
 * accompanied by other keys.
 *
 * For instance, media URI and and mime-type are related keys: mime-type does
 * not make sense if it is not accompanied by an URI. Moreover, for each URI
 * value, it has a correspondance mime-type value.
 *
 * #GrlProperty stores related keys and their values in one place, so user can
 * handle them in one shot.
 */

#include "grl-property.h"
#include "grl-log.h"

struct _GrlPropertyPrivate {
  GHashTable *data;
};

static void grl_property_finalize (GObject *object);
static void free_value (GValue *val);

#define GRL_PROPERTY_GET_PRIVATE(o)                                     \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GRL_TYPE_PROPERTY, GrlPropertyPrivate))

/* ================ GrlProperty GObject ================ */

G_DEFINE_TYPE (GrlProperty, grl_property, G_TYPE_OBJECT);

static void
grl_property_class_init (GrlPropertyClass *klass)
{
  GObjectClass *gobject_class = (GObjectClass *)klass;

  gobject_class->finalize = grl_property_finalize;

  g_type_class_add_private (klass, sizeof (GrlPropertyPrivate));
}

static void
grl_property_init (GrlProperty *self)
{
  self->priv = GRL_PROPERTY_GET_PRIVATE (self);
  self->priv->data = g_hash_table_new_full (g_direct_hash,
                                            g_direct_equal,
                                            NULL,
                                            (GDestroyNotify) free_value);
}

static void
grl_property_finalize (GObject *object)
{
  g_hash_table_unref (GRL_PROPERTY (object)->priv->data);
  G_OBJECT_CLASS (grl_property_parent_class)->finalize (object);
}

/* ================ Utitilies ================ */

static void
free_value (GValue *val)
{
  if (val) {
    g_value_unset (val);
    g_free (val);
  }
}

/* ================ API ================ */

/**
 * grl_property_new:
 *
 * Creates a new place to store related keys and values.
 *
 * Returns: a new object.
 **/
GrlProperty *
grl_property_new (void)
{
  return g_object_new (GRL_TYPE_PROPERTY, NULL);
}

/**
 * grl_property_new_valist:
 * @key: first key
 * @args: #va_list of value, followed by (key,value) pairs to insert
 *
 * Creates a new #GrlProperty containing pairs of (key, value). Finish the list with %NULL.
 *
 * In case of a binary-type key, the expected element is (key, value, size).
 *
 * value type will be extracted from key information.
 *
 * Returns: a new #GrlProperty
 **/
GrlProperty *
grl_property_new_valist (GrlKeyID key,
                         va_list args)
{
  GType key_type;
  GrlKeyID next_key;
  GrlProperty *prop;
  gpointer next_value;

  prop = grl_property_new ();

  next_key = key;
  while (next_key) {
    key_type = GRL_METADATA_KEY_GET_TYPE (next_key);
    if (key_type == G_TYPE_STRING) {
      grl_property_set_string (prop, next_key, va_arg (args, gchar *));
    } else if (key_type == G_TYPE_INT) {
      grl_property_set_int (prop, next_key, va_arg (args, gint));
    } else if (key_type == G_TYPE_FLOAT) {
      grl_property_set_float (prop, next_key, va_arg (args, double));
    } else if (key_type == G_TYPE_BYTE_ARRAY) {
      next_value = va_arg (args, gpointer);
      grl_property_set_binary (prop, next_key, next_value, va_arg (args, gsize));
    }
    next_key = va_arg (args, GrlKeyID);
  }

  return prop;
}

/**
 * grl_property_new_with_keys: (skip)
 * @key: first key
 * @...: value, following by list of (key, value)
 *
 * Creates a initial #GrlProperty containing the list of (key, value) pairs. Finish the list with %NULL.
 *
 * For more information see #grl_property_new_valist.

 * Returns: a new #GrlProperty
 **/
GrlProperty *
grl_property_new_with_keys (GrlKeyID key,
                            ...)
{
  GrlProperty *prop;
  va_list args;

  va_start (args, key);
  prop = grl_property_new_valist (key, args);
  va_end (args);

  return prop;
}


/**
 * grl_property_get:
 * @property: property to retrieve value
 * @key: (type Grl.KeyID): key to look up.
 *
 * Get the value associated with @key from @property. If it does not contain any
 * value, %NULL will be returned.
 *
 * Returns: (transfer none): a #GValue. This value should not be modified nor
 * freed by user.
 **/
const GValue *
grl_property_get (GrlProperty *property,
                  GrlKeyID key)
{
  g_return_val_if_fail (GRL_IS_PROPERTY (property), NULL);
  g_return_val_if_fail (key, NULL);

  return g_hash_table_lookup (property->priv->data, key);
}

/**
 * grl_property_set:
 * @property: property to modify
 * @key: (type Grl.KeyID): key to change or add
 * @value: the new value
 *
 * Sets the value associated with @key into @property. Old value is freed and
 * the new one is set.
 *
 * Also, checks that @value is compliant with @key specification, modifying it
 * accordingly. For instance, if @key requires a number between 0 and 10, but
 * value is outside this range, it will be adapted accordingly.
 **/
void
grl_property_set (GrlProperty *property,
                  GrlKeyID key,
                  const GValue *value)
{
  GValue *copy = NULL;

  g_return_if_fail (GRL_IS_PROPERTY (property));
  g_return_if_fail (key);

  /* Dup value */
  if (value) {
    if (G_VALUE_TYPE (value) == GRL_METADATA_KEY_GET_TYPE (key)) {
      copy = g_new0 (GValue, 1);
      g_value_init (copy, G_VALUE_TYPE (value));
      g_value_copy (value, copy);
    } else {
      GRL_WARNING ("value has type %s, but expected %s",
                   g_type_name (G_VALUE_TYPE (value)),
                   g_type_name (GRL_METADATA_KEY_GET_TYPE (key)));
    }
  }

  if (copy && g_param_value_validate (key, copy)) {
    GRL_WARNING ("'%s' value invalid, adjusting",
                 GRL_METADATA_KEY_GET_NAME (key));
  }
  g_hash_table_insert (property->priv->data, key, copy);
}

/**
 * grl_property_set_string:
 * @property: property to modify
 * @key: (type Grl.KeyID): key to change or add
 * @strvalue: the new value
 *
 * Sets the value associated with @key into @property. @key must have been
 * registered as a strying-type key. Old value is freed and the new one is set.
 **/
void
grl_property_set_string (GrlProperty *property,
                         GrlKeyID key,
                         const gchar *strvalue)
{
  if (strvalue) {
    GValue value = { 0 };
    g_value_init (&value, G_TYPE_STRING);
    g_value_set_string (&value, strvalue);
    grl_property_set (property, key, &value);
    g_value_unset (&value);
  } else {
    grl_property_set (property, key, NULL);
  }
}

/**
 * grl_property_get_string:
 * @property: property to inspect
 * @key: (type Grl.KeyID): key to use
 *
 * Returns the value associated with @key from @property. If @key has no value,
 * or value is not string, or @key is not in @property, then %NULL is returned.
 *
 * Returns: string associated with @key, or %NULL in other case. Caller should
 * not change nor free the value.
 **/
const gchar *
grl_property_get_string (GrlProperty *property,
                         GrlKeyID key)
{
  const GValue *value = grl_property_get (property, key);

  if (!value || !G_VALUE_HOLDS_STRING (value)) {
    return NULL;
  } else {
    return g_value_get_string (value);
  }
}

/**
 * grl_property_set_int:
 * @property: property to change
 * @key: (type Grl.KeyID): key to change or add
 * @intvalue: the new value
 *
 * Sets the value associated with @key into @property. @key must have been
 * registered as an int-type key. Old value is replaced by the new one.
 **/
void
grl_property_set_int (GrlProperty *property,
                      GrlKeyID key,
                      gint intvalue)
{
  GValue value = { 0 };
  g_value_init (&value, G_TYPE_INT);
  g_value_set_int (&value, intvalue);
  grl_property_set (property, key, &value);
}

/**
 * grl_property_get_int:
 * @property: property to inspect
 * @key: (type Grl.KeyID): key to use
 *
 * Returns the value associated with @key from @property. If @key has no value,
 * or value is not a gint, or @key is not in @property, then 0 is returned.
 *
 * Returns: int value associated with @key, or 0 in other case.
 **/
gint
grl_property_get_int (GrlProperty *property,
                      GrlKeyID key)
{
  const GValue *value = grl_property_get (property, key);

  if (!value || !G_VALUE_HOLDS_INT (value)) {
    return 0;
  } else {
    return g_value_get_int (value);
  }
}

/**
 * grl_property_set_float:
 * @property: property to change
 * @key: (type Grl.KeyID): key to change or add
 * @floatvalue: the new value
 *
 * Sets the value associated with @key into @property. @key must have been
 * registered as a float-type key. Old value is replaced by the new one.
 **/
void
grl_property_set_float (GrlProperty *property,
                        GrlKeyID key,
                        float floatvalue)
{
  GValue value = { 0 };
  g_value_init (&value, G_TYPE_FLOAT);
  g_value_set_float (&value, floatvalue);
  grl_property_set (property, key, &value);
}

/**
 * grl_property_get_float:
 * @property: property to inspect
 * @key: (type Grl.KeyID): key to use
 *
 * Returns the value associated with @key from @property. If @key has no value,
 * or value is not a gfloat, or @key is not in @property, then 0 is returned.
 *
 * Returns: float value associated with @key, or 0 in other case.
 **/
gfloat
grl_property_get_float (GrlProperty *property,
                        GrlKeyID key)
{
  const GValue *value = grl_property_get (property, key);

  if (!value || !G_VALUE_HOLDS_FLOAT (value)) {
    return 0;
  } else {
    return g_value_get_float (value);
  }
}

/**
 * grl_property_set_binary:
 * @property: property to change
 * @key: (type Grl.KeyID): key to change or add
 * @buf: buffer holding the property
 * @size: size of the buffer
 *
 * Sets the value associated with @key into @property. @key must have been
 * registered as a binary-type key. Old value is replaced by the new one.
 **/
void
grl_property_set_binary (GrlProperty *property,
                         GrlKeyID key,
                         const guint8 *buf,
                         gsize size)
{
  GValue v = { 0 };
  GByteArray *array;

  array = g_byte_array_append (g_byte_array_sized_new(size),
                               buf,
                               size);

  g_value_init (&v, g_byte_array_get_type ());
  g_value_take_boxed (&v, array);
  grl_property_set (property, key, &v);
  g_value_unset (&v);
}

/**
 * grl_property_get_binary:
 * @property: property to inspect
 * @key: (type Grl.KeyID): key to use
 * @size: (out): location to store the buffer size
 *
 * Returns the value associated with @key from @property. If @key has no value,
 * or value is not a binary, or @key is not in @property, then 0 is returned.
 *
 * Returns: buffer location associated with @key, or %NULL in other case. If
 * successful @size will be set to the buffer size.
 **/
const guint8 *
grl_property_get_binary (GrlProperty *property,
                         GrlKeyID key,
                         gsize *size)
{
  g_return_val_if_fail (size, NULL);

  const GValue *value = grl_property_get (property, key);

  if (!value || !G_VALUE_HOLDS_BOXED (value)) {
    return NULL;
  } else {
    GByteArray * array;

    array = g_value_get_boxed (value);
    *size = array->len;
    return (const guint8 *) array->data;
  }
}

/**
 * grl_property_add:
 * @property: property to change
 * @key: (type Grl.KeyID): key to add
 *
 * Adds a new @key to @property, with no value. If @key already exists, it does
 * nothing.
 **/
void
grl_property_add (GrlProperty *property,
                  GrlKeyID key)
{
  if (!grl_property_has_key (property, key)) {
    grl_property_set (property, key, NULL);
  }
}

/**
 * grl_property_remove:
 * @property: property to change
 * @key: (type Grl.KeyID): key to remove
 *
 * Removes @key from @property, freeing its value. If @key is not in @property,
 * then it does nothing.
 **/
void
grl_property_remove (GrlProperty *property, GrlKeyID key)
{
  g_return_if_fail (GRL_IS_PROPERTY (property));

  g_hash_table_remove (property->priv->data, key);
}

/**
 * grl_property_has_key:
 * @property: property to inspect
 * @key: (type Grl.KeyID): key to search
 *
 * Checks if @key is in @property.
 *
 * Returns: %TRUE if @key is in @property, %FALSE in other case.
 **/
gboolean
grl_property_has_key (GrlProperty *property,
                      GrlKeyID key)
{
  g_return_val_if_fail (GRL_IS_PROPERTY (property), FALSE);

  return g_hash_table_lookup_extended (property->priv->data, key, NULL, NULL);
}

/**
 * grl_property_get_keys:
 * @property: property to inspect
 * @include_unknown: %TRUE if keys with no value must be included
 *
 * Returns a list with keys contained in @property. If @include_unknown is
 * %FALSE, only those keys in @property that have actually a value will be
 * returned.
 *
 * Returns: (transfer container) (element-type Grl.KeyID): an array
 * with the keys. The content of the list should not be modified or freed. Use
 * g_list_free() when done using the list.
 **/
GList *
grl_property_get_keys (GrlProperty *property,
                       gboolean include_unknown)
{
  GList *keylist;

  g_return_val_if_fail (GRL_IS_PROPERTY (property), NULL);

  keylist = g_hash_table_get_keys (property->priv->data);

  if (!include_unknown) {
    keylist = g_list_remove_all (keylist, NULL);
  }

  return keylist;
}

/**
 * grl_property_key_is_known:
 * @property: property to inspect
 * @key: (type Grl.KeyID): key to search
 *
 * Checks if @key has a value in @property.
 *
 * Returns: %TRUE if @key has a value.
 **/
gboolean
grl_property_key_is_known (GrlProperty *property,
                           GrlKeyID key)
{
  GValue *v;

  g_return_val_if_fail (GRL_IS_PROPERTY (property), FALSE);

  v = g_hash_table_lookup (property->priv->data, key);

  if (!v) {
    return FALSE;
  }

  if (G_VALUE_HOLDS_STRING (v)) {
    return g_value_get_string (v) != NULL;
  }

  return TRUE;
}

/**
 * grl_property_dup:
 * @property: property to duplicate
 *
 * Makes a deep copy of @property and its contents.
 *
 * Returns: a new #GrlProperty. Free it with #g_object_unref.
 **/
GrlProperty *
grl_property_dup (GrlProperty *property)
{
  GList *keys, *key;
  const GValue *value;
  GValue *value_copy;
  GrlProperty *dup_property;

  g_return_val_if_fail (property, NULL);

  dup_property = grl_property_new ();

  keys = grl_property_get_keys (property, TRUE);
  for (key = keys; key; key = g_list_next (key)) {
    value = grl_property_get (property, key->data);
    if (value) {
      value_copy = g_new0 (GValue, 1);
      g_value_init (value_copy, G_VALUE_TYPE (value));
      g_value_copy (value, value_copy);
    } else {
      value_copy = NULL;
    }
    g_hash_table_insert (dup_property->priv->data, key->data, value_copy);
  }

  g_list_free (keys);

  return dup_property;
}
