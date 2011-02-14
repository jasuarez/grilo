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

#if !defined (_GRILO_H_INSIDE_) && !defined (GRILO_COMPILATION)
#error "Only <grilo.h> can be included directly."
#endif

#ifndef _GRL_MEDIA_CACHE_H_
#define _GRL_MEDIA_CACHE_H_

#include <glib.h>
#include <glib-object.h>

#include "grl-definitions.h"
#include "grl-media.h"

/* Macros */

#define GRL_TYPE_MEDIA_CACHE                    \
  (grl_media_cache_get_type ())

#define GRL_MEDIA_CACHE(obj)                                    \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj),                           \
                               GRL_TYPE_MEDIA_CACHE,            \
                               GrlMediaCache))

#define GRL_IS_MEDIA_CACHE(obj)                                 \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                           \
                               GRL_TYPE_MEDIA_CACHE))

#define GRL_MEDIA_CACHE_CLASS(klass)                    \
  (G_TYPE_CHECK_CLASS_CAST((klass),                     \
                           GRL_TYPE_MEDIA_CACHE,        \
                           GrlMediaCacheClass))

#define GRL_IS_MEDIA_CACHE_CLASS(klass)                 \
  (G_TYPE_CHECK_CLASS_TYPE((klass),                     \
                           GRL_TYPE_MEDIA_CACHE))

#define GRL_MEDIA_CACHE_GET_CLASS(obj)                  \
  (G_TYPE_INSTANCE_GET_CLASS ((obj),                    \
                              GRL_TYPE_MEDIA_CACHE,     \
                              GrlMediaCacheClass))

/* GrlMediaCache object */

typedef struct _GrlMediaCache        GrlMediaCache;
typedef struct _GrlMediaCachePrivate GrlMediaCachePrivate;
typedef struct _GrlMediaCacheClass   GrlMediaCacheClass;

struct _GrlMediaCache {

  GObject parent;

  /*< private >*/
  GrlMediaCachePrivate *priv;

  gpointer _grl_reserved[GRL_PADDING];
};

struct _GrlMediaCacheClass {
  GObjectClass parent_class;

  /*< private >*/
  gpointer _grl_reserved[GRL_PADDING];
};

G_BEGIN_DECLS

GType grl_media_cache_get_type (void);

GrlMediaCache *grl_media_cache_new (GList *keys);

GrlMediaCache *grl_media_cache_new_persistent (const gchar *cache_id, GList *keys);

GrlMediaCache *grl_media_cache_load_persistent (const gchar *cache_id);

void grl_media_cache_destroy (GrlMediaCache *cache);

gboolean grl_media_cache_insert_media (GrlMediaCache *cache,
                                       GrlMedia *media,
                                       const gchar *parent,
                                       GError **error);

GrlMedia *grl_media_cache_get_media (GrlMediaCache *cache,
                                     const gchar *media_id,
                                     gchar **parent,
                                     GTimeVal *last_time_changed,
                                     GError **error);

G_END_DECLS

#endif /* _GRL_MEDIA_CACHE_H_ */
