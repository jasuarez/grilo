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
 *
 */

#include "grl-property.h"
#include "grl-log.h"

G_DEFINE_TYPE (GrlProperty, grl_property, GRL_TYPE_DATA);

static void
grl_property_class_init (GrlPropertyClass *klass)
{
}

static void
grl_property_init (GrlProperty *self)
{
}

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
  GrlProperty *grlprop = g_object_new (GRL_TYPE_PROPERTY, NULL);

  g_object_set (grlprop, "overwrite", TRUE, NULL);

  return grlprop;
}

/**
 * grl_property_new_with_keys:
 * @related_keys: (element-type Grl.KeyID): list of keys.
 *
 * Creates a new place to store related keys and values.
 *
 * Initially it will contain the specified set of keys with no values.
 *
 * Returns: a new #GrlProperty
 **/
GrlProperty *
grl_property_new_with_keys (const GList *keys)
{
  GrlProperty *prop = grl_property_new ();

  while (keys) {
    grl_data_add (GRL_DATA (prop), keys->data);
    keys = g_list_next (keys);
  }

  return prop;
}
