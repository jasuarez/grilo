/*
 * Copyright (C) 2010 Igalia S.L.
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

#ifndef _GRL_MEDIA_SOURCE_H_
#define _GRL_MEDIA_SOURCE_H_

#include <grl-media-plugin.h>
#include <grl-metadata-source.h>
#include <grl-data.h>
#include <grl-media-box.h>

#include <glib.h>
#include <glib-object.h>

/* Macros */

#define GRL_TYPE_MEDIA_SOURCE                   \
  (grl_media_source_get_type ())

#define GRL_MEDIA_SOURCE(obj)                           \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj),                   \
                               GRL_TYPE_MEDIA_SOURCE,   \
                               GrlMediaSource))

#define GRL_IS_MEDIA_SOURCE(obj)                        \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                   \
                               GRL_TYPE_MEDIA_SOURCE))

#define GRL_MEDIA_SOURCE_CLASS(klass)                   \
  (G_TYPE_CHECK_CLASS_CAST((klass),                     \
                           GRL_TYPE_MEDIA_SOURCE,       \
                           GrlMediaSourceClass))

#define GRL_IS_MEDIA_SOURCE_CLASS(klass)                \
  (G_TYPE_CHECK_CLASS_TYPE((klass),                     \
                           GRL_TYPE_MEDIA_SOURCE))

#define GRL_MEDIA_SOURCE_GET_CLASS(obj)                         \
  (G_TYPE_INSTANCE_GET_CLASS ((obj),                            \
                              GRL_TYPE_MEDIA_SOURCE,            \
                              GrlMediaSourceClass))

/* GrlMediaSource object */

typedef struct _GrlMediaSource        GrlMediaSource;
typedef struct _GrlMediaSourcePrivate GrlMediaSourcePrivate;

struct _GrlMediaSource {

  GrlMetadataSource parent;

  /*< private >*/
  GrlMediaSourcePrivate *priv;
};

/* Callbacks for GrlMediaSource class */

/**
 * GrlMediaSourceResultCb:
 * @source: a media source
 * @operation_id: operation identifier
 * @media: a data transfer object
 * @remaining: the number of remaining #GrlMedia to process
 * @user_data: user data passed to the used method
 * @error: (not-error): possible #GError generated at processing
 *
 * Prototype for the callback passed to the media sources' methods
 */
typedef void (*GrlMediaSourceResultCb) (GrlMediaSource *source,
                                        guint operation_id,
                                        GrlMedia *media,
                                        guint remaining,
                                        gpointer user_data,
                                        const GError *error);

/**
 * GrlMediaSourceMetadataCb:
 * @source: a media source
 * @media: a data transfer object
 * @user_data: user data passed to grl_media_source_metadata()
 * @error: (not-error): possible #GError generated at processing
 *
 * Prototype for the callback passed to grl_media_source_metadata()
 */
typedef void (*GrlMediaSourceMetadataCb) (GrlMediaSource *source,
                                          GrlMedia *media,
                                          gpointer user_data,
                                          const GError *error);

/**
 * GrlMediaSourceStoreCb:
 * @source: a media source
 * @parent: TBD
 * @media: a data transfer object
 * @user_data: user data passed to grl_media_source_store()
 * @error: (not-error): possible #GError generated at processing
 *
 * Prototype for the callback passed to grl_media_source_store()
 */
typedef void (*GrlMediaSourceStoreCb) (GrlMediaSource *source,
                                       GrlMediaBox *parent,
                                       GrlMedia *media,
                                       gpointer user_data,
                                       const GError *error);

/**
 * GrlMediaSourceRemoveCb:
 * @source: a media source
 * @media: a data transfer object
 * @user_data: user data passed to grl_media_source_remove()
 * @error: (not-error): possible #GError generated at processing
 *
 * Prototype for the callback passed to grl_media_source_remove()
 */
typedef void (*GrlMediaSourceRemoveCb) (GrlMediaSource *source,
                                        GrlMedia *media,
                                        gpointer user_data,
                                        const GError *error);

/* Types for MediaSourceClass */

/**
 * GrlMediaSourceBrowseSpec:
 * @source: a media source
 * @browse_id: operation identifier
 * @container: a container of data transfer objects
 * @keys: the list of #GrlKeyID to request
 * @skip: the number if elements to skip in the browse operation
 * @count: the number of elements to retrieve in the browse operation
 * @flags: the resolution mode
 * @callback: the user defined callback
 * @user_data: the user data to pass in the callback
 * @ref_count: references counter
 *
 * Data transport structure used internally by the plugins which support
 * browse vmethod.
 */
typedef struct {
  GrlMediaSource *source;
  guint browse_id;
  GrlMedia *container;
  GList *keys;
  guint skip;
  guint count;
  GrlMetadataResolutionFlags flags;
  GrlMediaSourceResultCb callback;
  gpointer user_data;
  volatile gint ref_count;
} GrlMediaSourceBrowseSpec;

#define grl_media_source_browse_spec_get_source(bs)    ((bs)->source)
#define grl_media_source_browse_spec_get_id(bs)        ((bs)->browse_id)
#define grl_media_source_browse_spec_get_container(bs) ((bs)->container)
#define grl_media_source_browse_spec_get_keys(bs)      ((bs)->keys)
#define grl_media_source_browse_spec_get_skip(bs)      ((bs)->skip)
#define grl_media_source_browse_spec_get_count(bs)     ((bs)->count)
#define grl_media_source_browse_spec_get_flags(bs)     ((bs)->flags)
#define grl_media_source_browse_spec_get_callback(bs)  ((bs)->callback)
#define grl_media_source_browse_spec_get_data(bs)      ((bs)->user_data)

#define grl_media_source_browse_spec_set_source(bs, _source)       ((bs)->source = g_object_ref(_source))
#define grl_media_source_browse_spec_set_id(bs, _id)               ((bs)->browse_id = (_id))
#define grl_media_source_browse_spec_set_container(bs, _container) ((bs)->container = g_object_ref(_container))
#define grl_media_source_browse_spec_set_keys(bs, _keys)           ((bs)->keys = (_keys))
#define grl_media_source_browse_spec_set_skip(bs, _skip)           ((bs)->skip = (_skip))
#define grl_media_source_browse_spec_set_count(bs, _count)         ((bs)->count = (_count))
#define grl_media_source_browse_spec_set_flags(bs, _flags)         ((bs)->flags = (_flags))
#define grl_media_source_browse_spec_set_callback(bs, _callback)   ((bs)->callback = (_callback))
#define grl_media_source_browse_spec_set_data(bs, _data)           ((bs)->user_data = (_data))

/**
 * GrlMediaSourceSearchSpec:
 * @source: a media source
 * @search_id: operation identifier
 * @text: the text to search
 * @keys: the list of #GrlKeyID to request
 * @skip: the number if elements to skip in the browse operation
 * @count: the number of elements to retrieve in the browse operation
 * @flags: the resolution mode
 * @callback: the user defined callback
 * @user_data: the user data to pass in the callback
 * @ref_count: references counter
 *
 * Data transport structure used internally by the plugins which support
 * search vmethod.
 */
typedef struct {
  GrlMediaSource *source;
  guint search_id;
  gchar *text;
  GList *keys;
  guint skip;
  guint count;
  GrlMetadataResolutionFlags flags;
  GrlMediaSourceResultCb callback;
  gpointer user_data;
  volatile gint ref_count;
} GrlMediaSourceSearchSpec;

#define grl_media_source_search_spec_get_source(ss)   ((ss)->source)
#define grl_media_source_search_spec_get_id(ss)       ((ss)->search_id)
#define grl_media_source_search_spec_get_text(ss)     ((ss)->text)
#define grl_media_source_search_spec_get_keys(ss)     ((ss)->keys)
#define grl_media_source_search_spec_get_skip(ss)     ((ss)->skip)
#define grl_media_source_search_spec_get_count(ss)    ((ss)->count)
#define grl_media_source_search_spec_get_flags(ss)    ((ss)->flags)
#define grl_media_source_search_spec_get_callback(ss) ((ss)->callback)
#define grl_media_source_search_spec_get_data(ss)     ((ss)->user_data)

#define grl_media_source_search_spec_set_source(ss, _source)     ((ss)->source = g_object_ref(_source))
#define grl_media_source_search_spec_set_id(ss, _id)             ((ss)->search_id = (_id))
#define grl_media_source_search_spec_set_text(ss, _text)         ((ss)->text = g_strdup(_text))
#define grl_media_source_search_spec_set_keys(ss, _keys)         ((ss)->keys = (_keys))
#define grl_media_source_search_spec_set_skip(ss, _skip)         ((ss)->skip = (_skip))
#define grl_media_source_search_spec_set_count(ss, _count)       ((ss)->count = (_count))
#define grl_media_source_search_spec_set_flags(ss, _flags)       ((ss)->flags = (_flags))
#define grl_media_source_search_spec_set_callback(ss, _callback) ((ss)->callback = (_callback))
#define grl_media_source_search_spec_set_data(ss, _data)         ((ss)->user_data = (_data))

/**
 * GrlMediaSourceQuerySpec:
 * @source: a media source
 * @query_id: operation identifier
 * @query: the query to process
 * @keys: the list of #GrlKeyID to request
 * @skip: the number if elements to skip in the browse operation
 * @count: the number of elements to retrieve in the browse operation
 * @flags: the resolution mode
 * @callback: the user defined callback
 * @user_data: the user data to pass in the callback
 * @ref_count: references counter
 *
 * Data transport structure used internally by the plugins which support
 * query vmethod.
 */
typedef struct {
  GrlMediaSource *source;
  guint query_id;
  gchar *query;
  GList *keys;
  guint skip;
  guint count;
  GrlMetadataResolutionFlags flags;
  GrlMediaSourceResultCb callback;
  gpointer user_data;
  volatile gint ref_count;
} GrlMediaSourceQuerySpec;

#define grl_media_source_query_spec_get_source(qs)   ((qs)->source)
#define grl_media_source_query_spec_get_id(qs)       ((qs)->query_id)
#define grl_media_source_query_spec_get_query(qs)    ((qs)->query)
#define grl_media_source_query_spec_get_keys(qs)     ((qs)->keys)
#define grl_media_source_query_spec_get_skip(qs)     ((qs)->skip)
#define grl_media_source_query_spec_get_count(qs)    ((qs)->count)
#define grl_media_source_query_spec_get_flags(qs)    ((qs)->flags)
#define grl_media_source_query_spec_get_callback(qs) ((qs)->callback)
#define grl_media_source_query_spec_get_data(qs)     ((qs)->user_data)

#define grl_media_source_query_spec_set_source(qs, _source)     ((qs)->source = g_object_ref(_source))
#define grl_media_source_query_spec_set_id(qs, _id)             ((qs)->query_id = (_id))
#define grl_media_source_query_spec_set_query(qs, _query)       ((qs)->query = g_strdup(_query))
#define grl_media_source_query_spec_set_keys(qs, _keys)         ((qs)->keys = (_keys))
#define grl_media_source_query_spec_set_skip(qs, _skip)         ((qs)->skip = (_skip))
#define grl_media_source_query_spec_set_count(qs, _count)       ((qs)->count = (_count))
#define grl_media_source_query_spec_set_flags(qs, _flags)       ((qs)->flags = (_flags))
#define grl_media_source_query_spec_set_callback(qs, _callback) ((qs)->callback = (_callback))
#define grl_media_source_query_spec_set_data(qs, _data)         ((qs)->user_data = (_data))

/**
 * GrlMediaSourceMetadataSpec:
 * @source: a media source
 * @metadata_id: operation identifier
 * @media: a data transfer object
 * @keys: the list of #GrlKeyID to request
 * @flags: the resolution mode
 * @callback: the user defined callback
 * @user_data: the user data to pass in the callback
 * @ref_count: references counter
 *
 * Data transport structure used internally by the plugins which support
 * metadata vmethod.
 */
typedef struct {
  GrlMediaSource *source;
  guint metadata_id;
  GrlMedia *media;
  GList *keys;
  GrlMetadataResolutionFlags flags;
  GrlMediaSourceMetadataCb callback;
  gpointer user_data;
  volatile gint ref_count;
} GrlMediaSourceMetadataSpec;

#define grl_media_source_metadata_spec_get_source(ms)   ((ms)->source)
#define grl_media_source_metadata_spec_get_media(ms)    ((ms)->media)
#define grl_media_source_metadata_spec_get_keys(ms)     ((ms)->keys)
#define grl_media_source_metadata_spec_get_flags(ms)    ((ms)->flags)
#define grl_media_source_metadata_spec_get_callback(ms) ((ms)->callback)
#define grl_media_source_metadata_spec_get_data(ms)     ((ms)->user_data)

#define grl_media_source_metadata_spec_set_source(ms, _source)     ((ms)->source = g_object_ref(_source))
#define grl_media_source_metadata_spec_set_media(ms, _media)       ((ms)->media = g_object_ref(_media))
#define grl_media_source_metadata_spec_set_keys(ms, _keys)         ((ms)->keys = (_keys))
#define grl_media_source_metadata_spec_set_flags(ms, _flags)       ((ms)->flags = (_flags))
#define grl_media_source_metadata_spec_set_callback(ms, _callback) ((ms)->callback = (_callback))
#define grl_media_source_metadata_spec_set_data(ms, _data)         ((ms)->user_data = (_data))

/**
 * GrlMediaSourceStoreSpec:
 * @source: a media source
 * @parent: a parent to store the data transfer objects
 * @media: a data transfer object
 * @callback: the user defined callback
 * @user_data: the user data to pass in the callback
 * @ref_count: references counter
 *
 * Data transport structure used internally by the plugins which support
 * store vmethod.
 */
typedef struct {
  GrlMediaSource *source;
  GrlMediaBox *parent;
  GrlMedia *media;
  GrlMediaSourceStoreCb callback;
  gpointer user_data;
  volatile gint ref_count;
} GrlMediaSourceStoreSpec;

#define grl_media_source_store_spec_get_source(ss)   ((ss)->source)
#define grl_media_source_store_spec_get_parent(ss)   ((ss)->parent)
#define grl_media_source_store_spec_get_media(ss)    ((ss)->media)
#define grl_media_source_store_spec_get_callback(ss) ((ss)->callback)
#define grl_media_source_store_spec_get_data(ss)     ((ss)->user_data)

#define grl_media_source_store_spec_set_source(ss, _source)     ((ss)->source = g_object_ref(_source))
#define grl_media_source_store_spec_set_parent(ss, _parent)     ((ss)->parent = (_parent)?g_object_ref(_parent):NULL)
#define grl_media_source_store_spec_set_media(ss, _media)       ((ss)->media = g_object_ref(_media))
#define grl_media_source_store_spec_set_callback(ss, _callback) ((ss)->callback = (_callback))
#define grl_media_source_store_spec_set_data(ss, _data)         ((ss)->user_data = (_data))

/**
 * GrlMediaSourceRemoveSpec:
 * @source: a media source
 * @media_id: media identifier to remove
 * @media: a data transfer object
 * @callback: the user defined callback
 * @user_data: the user data to pass in the callback
 * @ref_count: references counter
 *
 * Data transport structure used internally by the plugins which support
 * store vmethod.
 */
typedef struct {
  GrlMediaSource *source;
  gchar *media_id;
  GrlMedia *media;
  GrlMediaSourceRemoveCb callback;
  gpointer user_data;
  volatile gint ref_count;
} GrlMediaSourceRemoveSpec;

#define grl_media_source_remove_spec_get_source(rs)   ((rs)->source)
#define grl_media_source_remove_spec_get_media_id(rs) ((rs)->media_id)
#define grl_media_source_remove_spec_get_media(rs)    ((rs)->media)
#define grl_media_source_remove_spec_get_callback(rs) ((rs)->callback)
#define grl_media_source_remove_spec_get_data(rs)     ((rs)->user_data)

#define grl_media_source_remove_spec_set_source(rs, _source)     ((rs)->source = g_object_ref(_source))
#define grl_media_source_remove_spec_set_media_id(rs, _media_id) ((rs)->media_id = g_strdup(_media_id))
#define grl_media_source_remove_spec_set_media(rs, _media)       ((rs)->media = g_object_ref(_media))
#define grl_media_source_remove_spec_set_callback(rs, _callback) ((rs)->callback = (_callback))
#define grl_media_source_remove_spec_set_data(rs, _data)         ((rs)->user_data = (_data))

/* GrlMediaSource class */

typedef struct _GrlMediaSourceClass GrlMediaSourceClass;

/**
 * GrlMediaSourceClass:
 * @parent_class: the parent class structure
 * @operation_id: operation identifier
 * @browse: browse through a list of media
 * @search: search for media
 * @query: query for a specific media
 * @cancel: cancel the current operation
 * @metadata: request for specific metadata
 * @store: store a media in a container
 * @remove: remove a media from a container
 *
 * Grilo MediaSource class. Override the vmethods to implement the
 * source functionality.
 */
struct _GrlMediaSourceClass {

  GrlMetadataSourceClass parent_class;

  guint operation_id;

  void (*browse) (GrlMediaSource *source, GrlMediaSourceBrowseSpec *bs);

  void (*search) (GrlMediaSource *source, GrlMediaSourceSearchSpec *ss);

  void (*query) (GrlMediaSource *source, GrlMediaSourceQuerySpec *qs);

  void (*cancel) (GrlMediaSource *source, guint operation_id);

  void (*metadata) (GrlMediaSource *source, GrlMediaSourceMetadataSpec *ms);

  void (*store) (GrlMediaSource *source, GrlMediaSourceStoreSpec *ss);

  void (*remove) (GrlMediaSource *source, GrlMediaSourceRemoveSpec *ss);
};

G_BEGIN_DECLS

GType grl_media_source_get_type (void);

GrlMediaSourceBrowseSpec *grl_media_source_browse_spec_new (void);

GrlMediaSourceSearchSpec *grl_media_source_search_spec_new (void);

GrlMediaSourceQuerySpec *grl_media_source_query_spec_new (void);

GrlMediaSourceMetadataSpec *grl_media_source_metadata_spec_new (void);

GrlMediaSourceStoreSpec *grl_media_source_store_spec_new (void);

GrlMediaSourceRemoveSpec *grl_media_source_remove_spec_new (void);

GrlMediaSourceBrowseSpec *grl_media_source_browse_spec_ref (GrlMediaSourceBrowseSpec *bs);

GrlMediaSourceSearchSpec *grl_media_source_search_spec_ref (GrlMediaSourceSearchSpec *ss);

GrlMediaSourceQuerySpec *grl_media_source_query_spec_ref (GrlMediaSourceQuerySpec *qs);

GrlMediaSourceMetadataSpec *grl_media_source_metadata_spec_ref (GrlMediaSourceMetadataSpec *ms);

GrlMediaSourceStoreSpec *grl_media_source_store_spec_ref (GrlMediaSourceStoreSpec *ss);

GrlMediaSourceRemoveSpec *grl_media_source_remove_spec_ref (GrlMediaSourceRemoveSpec *rs);

void grl_media_source_browse_spec_unref (GrlMediaSourceBrowseSpec *bs);

void grl_media_source_search_spec_unref (GrlMediaSourceSearchSpec *bs);

void grl_media_source_query_spec_unref (GrlMediaSourceQuerySpec *bs);

void grl_media_source_metadata_spec_unref (GrlMediaSourceMetadataSpec *bs);

void grl_media_source_store_spec_unref (GrlMediaSourceStoreSpec *bs);

void grl_media_source_remove_spec_unref (GrlMediaSourceRemoveSpec *bs);

guint grl_media_source_browse (GrlMediaSource *source,
                               GrlMedia *container,
                               const GList *keys,
                               guint skip,
                               guint count,
                               GrlMetadataResolutionFlags flags,
                               GrlMediaSourceResultCb callback,
                               gpointer user_data);

GList *grl_media_source_browse_sync (GrlMediaSource *source,
                                     GrlMedia *container,
                                     const GList *keys,
                                     guint skip,
                                     guint count,
                                     GrlMetadataResolutionFlags flags,
                                     GError **error);

guint grl_media_source_search (GrlMediaSource *source,
                               const gchar *text,
                               const GList *keys,
                               guint skip,
                               guint count,
                               GrlMetadataResolutionFlags flags,
                               GrlMediaSourceResultCb callback,
                               gpointer user_data);

GList *grl_media_source_search_sync (GrlMediaSource *source,
                                     const gchar *text,
                                     const GList *keys,
                                     guint skip,
                                     guint count,
                                     GrlMetadataResolutionFlags flags,
                                     GError **error);

guint grl_media_source_query (GrlMediaSource *source,
                              const gchar *query,
                              const GList *keys,
                              guint skip,
                              guint count,
                              GrlMetadataResolutionFlags flags,
                              GrlMediaSourceResultCb callback,
                              gpointer user_data);

GList *grl_media_source_query_sync (GrlMediaSource *source,
                                    const gchar *query,
                                    const GList *keys,
                                    guint skip,
                                    guint count,
                                    GrlMetadataResolutionFlags flags,
                                    GError **error);

guint grl_media_source_metadata (GrlMediaSource *source,
                                 GrlMedia *media,
                                 const GList *keys,
                                 GrlMetadataResolutionFlags flags,
                                 GrlMediaSourceMetadataCb callback,
                                 gpointer user_data);

GrlMedia *grl_media_source_metadata_sync (GrlMediaSource *source,
                                          GrlMedia *media,
                                          const GList *keys,
                                          GrlMetadataResolutionFlags flags,
                                          GError **error);

void grl_media_source_store (GrlMediaSource *source,
                             GrlMediaBox *parent,
                             GrlMedia *media,
                             GrlMediaSourceStoreCb callback,
                             gpointer user_data);

void grl_media_source_store_sync (GrlMediaSource *source,
                                  GrlMediaBox *parent,
                                  GrlMedia *media,
                                  GError **error);

void grl_media_source_remove (GrlMediaSource *source,
                              GrlMedia *media,
                              GrlMediaSourceRemoveCb callback,
                              gpointer user_data);

void grl_media_source_remove_sync (GrlMediaSource *source,
                                   GrlMedia *media,
                                   GError **error);


void grl_media_source_cancel (GrlMediaSource *source, guint operation_id);

void grl_media_source_set_operation_data (GrlMediaSource *source,
                                          guint operation_id,
                                          gpointer data);

gpointer grl_media_source_get_operation_data (GrlMediaSource *source,
                                              guint operation_id);

void grl_media_source_set_auto_split_threshold (GrlMediaSource *source,
                                                guint threshold);

guint grl_media_source_get_auto_split_threshold (GrlMediaSource *source);

G_END_DECLS

#endif /* _GRL_MEDIA_SOURCE_H_ */
