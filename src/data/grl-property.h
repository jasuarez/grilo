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

#ifndef _GRL_PROPERTY_H_
#define _GRL_PROPERTY_H_

#include <glib-object.h>
#include <grl-metadata-key.h>
#include <grl-definitions.h>

G_BEGIN_DECLS

#define GRL_TYPE_PROPERTY                       \
  (grl_property_get_type())

#define GRL_PROPERTY(obj)                               \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj),                   \
                               GRL_TYPE_PROPERTY,       \
                               GrlProperty))

#define GRL_PROPERTY_CLASS(klass)               \
  (G_TYPE_CHECK_CLASS_CAST ((klass),            \
                            GRL_TYPE_PROPERTY,  \
                            GrlPropertyClass))

#define GRL_IS_PROPERTY(obj)                            \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                   \
                               GRL_TYPE_PROPERTY))

#define GRL_IS_PROPERTY_CLASS(klass)            \
  (G_TYPE_CHECK_CLASS_TYPE ((klass),            \
                            GRL_TYPE_PROPERTY))

#define GRL_PROPERTY_GET_CLASS(obj)                     \
  (G_TYPE_INSTANCE_GET_CLASS ((obj),                    \
                              GRL_TYPE_PROPERTY,        \
                              GrlPropertyClass))

typedef struct _GrlProperty        GrlProperty;
typedef struct _GrlPropertyPrivate GrlPropertyPrivate;
typedef struct _GrlPropertyClass   GrlPropertyClass;

/**
 * GrlPropertyClass:
 * @parent_class: the parent class structure
 *
 * Grilo Data Multivalued class
 */
struct _GrlPropertyClass
{
  GObjectClass parent_class;

  /*< private >*/
  gpointer _grl_reserved[GRL_PADDING];
};

struct _GrlProperty
{
  GObject parent;

  /*< private >*/
  GrlPropertyPrivate *priv;

  gpointer _grl_reserved[GRL_PADDING_SMALL];
};

GType grl_property_get_type (void) G_GNUC_CONST;

GrlProperty *grl_property_new (void);

GrlProperty *grl_property_new_for_key (GrlKeyID key);

void grl_property_set (GrlProperty *property,
                       GrlKeyID key,
                       const GValue *value);

void grl_property_set_string (GrlProperty *property,
                              GrlKeyID key,
                              const gchar *strvalue);

void grl_property_set_int (GrlProperty *property,
                           GrlKeyID key,
                           gint intvalue);

void grl_property_set_float (GrlProperty *property,
                             GrlKeyID key,
                             gfloat floatvalue);

void grl_property_set_binary(GrlProperty *property,
                             GrlKeyID key,
                             const guint8 *buf,
                             gsize size);

const GValue *grl_property_get (GrlProperty *property,
                                GrlKeyID key);

const gchar *grl_property_get_string (GrlProperty *property,
                                      GrlKeyID key);

gint grl_property_get_int (GrlProperty *property,
                           GrlKeyID key);

gfloat grl_property_get_float (GrlProperty *property,
                               GrlKeyID key);

const guint8 *grl_property_get_binary(GrlProperty *property,
                                      GrlKeyID key,
                                      gsize *size);

void grl_property_add (GrlProperty *property,
                       GrlKeyID key);

void grl_property_remove (GrlProperty *property,
                          GrlKeyID key);

gboolean grl_property_has_key (GrlProperty *property,
                               GrlKeyID key);

GList *grl_property_get_keys (GrlProperty *property,
                              gboolean include_unknown);

gboolean grl_property_key_is_known (GrlProperty *property,
                                    GrlKeyID key);

GrlProperty *grl_property_dup (GrlProperty *property);

G_END_DECLS

#endif /* _GRL_PROPERTY_H_ */
