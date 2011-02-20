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

#if !defined (_GRILO_H_INSIDE_) && !defined (GRILO_COMPILATION)
#error "Only <grilo.h> can be included directly."
#endif

#ifndef _GRL_DATA_MULTI_H_
#define _GRL_DATA_MULTI_H_

#include <grl-data.h>
#include <grl-property.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define GRL_TYPE_DATA_MULTI                     \
  (grl_data_multi_get_type())

#define GRL_DATA_MULTI(obj)                           \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj),                 \
                               GRL_TYPE_DATA_MULTI,   \
                               GrlDataMulti))

#define GRL_DATA_MULTI_CLASS(klass)                   \
  (G_TYPE_CHECK_CLASS_CAST ((klass),                  \
                            GRL_TYPE_DATA_MULTI,      \
                            GrlDataMultiClass))

#define GRL_IS_DATA_MULTI(obj)                          \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                   \
                               GRL_TYPE_DATA_MULTI))

#define GRL_IS_DATA_MULTI_CLASS(klass)                  \
  (G_TYPE_CHECK_CLASS_TYPE ((klass),                    \
                            GRL_TYPE_DATA_MULTI))

#define GRL_DATA_MULTI_GET_CLASS(obj)                   \
  (G_TYPE_INSTANCE_GET_CLASS ((obj),                    \
                              GRL_TYPE_DATA_MULTI,      \
                              GrlDataMultiClass))

typedef struct _GrlDataMulti        GrlDataMulti;
typedef struct _GrlDataMultiPrivate GrlDataMultiPrivate;
typedef struct _GrlDataMultiClass   GrlDataMultiClass;

/**
 * GrlDataMultiClass:
 * @parent_class: the parent class structure
 *
 * Grilo Data Multivalued class
 */
struct _GrlDataMultiClass
{
  GrlDataClass parent_class;

  /*< private >*/
  gpointer _grl_reserved[GRL_PADDING];
};

struct _GrlDataMulti
{
  GrlData parent;

  /*< private >*/
  GrlDataMultiPrivate *priv;

  gpointer _grl_reserved[GRL_PADDING_SMALL];
};

GType grl_data_multi_get_type (void) G_GNUC_CONST;

GrlDataMulti *grl_data_multi_new (void);

void grl_data_multi_add (GrlDataMulti *mdata, GrlProperty *prop);

guint grl_data_multi_length (GrlDataMulti *mdata, GrlKeyID key);

GrlProperty *grl_data_multi_get (GrlDataMulti *mdata, GrlKeyID key, guint pos);

GList *grl_data_multi_get_all_single (GrlDataMulti *mdata, GrlKeyID key);

GList *grl_data_multi_get_all_single_string (GrlDataMulti *mdata, GrlKeyID key);

void grl_data_multi_remove (GrlDataMulti *mdata, GrlKeyID key, guint pos);

void grl_data_multi_update (GrlDataMulti *mdata, GrlProperty *prop, guint pos);

G_END_DECLS

#endif /* _GRL_DATA_MULTI_H_ */
