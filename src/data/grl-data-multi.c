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

struct _GrlDataMultiPrivate {
  GHashTable *extended_data;
};

static void grl_data_multi_finalize (GObject *object);

#define GRL_DATA_MULTI_GET_PRIVATE(o)                                   \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GRL_TYPE_DATA_MULTI, GrlDataMultiPrivate))

G_DEFINE_TYPE (GrlDataMulti, grl_data_multi, GRL_TYPE_DATA);

static void
free_list_values (GrlKeyID key,
                  GList *values,
                  gpointer user_data)
{
  g_list_foreach (values, (GFunc) g_object_unref, NULL);
  g_list_free (values);
}


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
