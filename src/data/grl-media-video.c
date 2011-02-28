/*
 * Copyright (C) 2010, 2011 Igalia S.L.
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
 * SECTION:grl-media-video
 * @short_description: A multimedia data for video
 * @see_also: #GrlConfig, #GrlMediaBox, #GrlMediaAudio, #GrlMediaImage
 *
 * This high level class represents an video multimedia item. It has methods to
 * set and get properties like framerate, width, height, and so on.
 */

#include "grl-media-video.h"


static void grl_media_video_dispose (GObject *object);
static void grl_media_video_finalize (GObject *object);

G_DEFINE_TYPE (GrlMediaVideo, grl_media_video, GRL_TYPE_MEDIA);

static void
grl_media_video_class_init (GrlMediaVideoClass *klass)
{
  GObjectClass *gobject_class = (GObjectClass *)klass;

  gobject_class->dispose = grl_media_video_dispose;
  gobject_class->finalize = grl_media_video_finalize;
}

static void
grl_media_video_init (GrlMediaVideo *self)
{
}

static void
grl_media_video_dispose (GObject *object)
{
  G_OBJECT_CLASS (grl_media_video_parent_class)->dispose (object);
}

static void
grl_media_video_finalize (GObject *object)
{
  g_signal_handlers_destroy (object);
  G_OBJECT_CLASS (grl_media_video_parent_class)->finalize (object);
}

/**
 * grl_media_video_new:
 *
 * Creates a new data video object.
 *
 * Returns: a newly-allocated data video.
 *
 * Since: 0.1.4
 */
GrlMedia *
grl_media_video_new (void)
{
  return GRL_MEDIA (g_object_new (GRL_TYPE_MEDIA_VIDEO,
                                  NULL));
}

/**
 * grl_media_video_set_size:
 * @video: the media instance
 * @width: the video's width
 * @height: the video's height
 *
 * Set the width and the height of the video
 *
 * Since: 0.1.4
 */
void
grl_media_video_set_size (GrlMediaVideo *video,
                          gint width,
                          int height)
{
  grl_media_video_set_width (video, width);
  grl_media_video_set_height (video, height);
}

/**
 * grl_media_video_set_width:
 * @data: the media instance
 * @width: the video's width
 *
 * Set the width of the video
 *
 * Since: 0.1.4
 */
void
grl_media_video_set_width (GrlMediaVideo *data, gint width)
{
  grl_data_set_int (GRL_DATA (data),
                    GRL_METADATA_KEY_WIDTH,
                    width);
}

/**
 * grl_media_video_set_height:
 * @data: the media instance
 * @height: the video's height
 *
 * Set the height of the video
 *
 * Since: 0.1.4
 */
void
grl_media_video_set_height (GrlMediaVideo *data, gint height)
{
  grl_data_set_int (GRL_DATA (data),
                    GRL_METADATA_KEY_HEIGHT,
                    height);
}

/**
 * grl_media_video_set_framerate:
 * @data: the media instance
 * @framerate: the video's framerate
 *
 * Set the framerate of the video
 *
 * Since: 0.1.4
 */
void
grl_media_video_set_framerate (GrlMediaVideo *data, gfloat framerate)
{
  grl_data_set_float (GRL_DATA (data),
                      GRL_METADATA_KEY_FRAMERATE,
                      framerate);
}

/**
 * grl_media_video_get_width:
 * @data: the media instance
 *
 * Returns: the width of the video
 *
 * Since: 0.1.4
 */
gint
grl_media_video_get_width (GrlMediaVideo *data)
{
  return grl_data_get_int (GRL_DATA (data), GRL_METADATA_KEY_WIDTH);
}

/**
 * grl_media_video_get_height:
 * @data: the media instance
 *
 * Returns: the height of the video
 *
 * Since: 0.1.4
 */
gint
grl_media_video_get_height (GrlMediaVideo *data)
{
  return grl_data_get_int (GRL_DATA (data), GRL_METADATA_KEY_HEIGHT);
}

/**
 * grl_media_video_get_framerate:
 * @data: the media instance
 *
 * Returns: the framerate of the video
 *
 * Since: 0.1.4
 */
gfloat
grl_media_video_get_framerate (GrlMediaVideo *data)
{
  return grl_data_get_float (GRL_DATA (data), GRL_METADATA_KEY_FRAMERATE);
}

/**
 * grl_media_video_set_url_data:
 * @video: the media instance
 * @url: the video's url
 * @mime: video mime-type
 * @framerate: video framerate, or -1 to ignore
 * @width: video width, or -1 to ignore
 * @height: video height, or -1 to ignore
 *
 * Sets the video url, as well as its mime type, framerate, width and height.
 **/
void
grl_media_video_set_url_data (GrlMediaVideo *video,
                              const gchar *url,
                              const gchar *mime,
                              gfloat framerate,
                              gint width,
                              gint height)
{
  GrlProperty *prop = grl_property_new_for_key (GRL_METADATA_KEY_URL);
  grl_property_set_string (prop, GRL_METADATA_KEY_URL, url);
  grl_property_set_string (prop, GRL_METADATA_KEY_MIME, mime);
  if (framerate >= 0) {
    grl_property_set_float (prop, GRL_METADATA_KEY_FRAMERATE, framerate);
  }
  if (width >= 0) {
    grl_property_set_int (prop, GRL_METADATA_KEY_WIDTH, width);
  }
  if (height >= 0) {
    grl_property_set_int (prop, GRL_METADATA_KEY_HEIGHT, height);
  }
  grl_data_set_property (GRL_DATA (video), prop, 0);
}

/**
 * grl_media_video_add_url_data:
 * @video: the media instance
 * @url: a video's url
 * @mime: video mime-type
 * @framerate: video framerate, or -1 to ignore
 * @width: video width, or -1 to ignore
 * @height: video height, or -1 to ignore
 *
 * Sets a new video url, as well as its mime type, framerate, width and height.
 **/
void
grl_media_video_add_url_data (GrlMediaVideo *video,
                              const gchar *url,
                              const gchar *mime,
                              gfloat framerate,
                              gint width,
                              gint height)
{
  GrlProperty *prop = grl_property_new_for_key (GRL_METADATA_KEY_URL);
  grl_property_set_string (prop, GRL_METADATA_KEY_URL, url);
  grl_property_set_string (prop, GRL_METADATA_KEY_MIME, mime);
  if (framerate >= 0) {
    grl_property_set_float (prop, GRL_METADATA_KEY_FRAMERATE, framerate);
  }
  if (width >= 0) {
    grl_property_set_int (prop, GRL_METADATA_KEY_WIDTH, width);
  }
  if (height >= 0) {
    grl_property_set_int (prop, GRL_METADATA_KEY_HEIGHT, height);
  }
  grl_data_add_property (GRL_DATA (video), prop);
}

/**
 * grl_media_video_get_url_data:
 * @video: the media instance
 * @mime: (out) (transfer none): the url mime-type, or %NULL to ignore
 * @framerate: the url framerate, or %NULL to ignore
 * @width: the url width, or %NULL to ignore
 * @height: the url height, or %NULL to ignore
 *
 * Returns: the video's url, as well as its mime-type, framerate, width and height.
 **/
const gchar *
grl_media_video_get_url_data (GrlMediaVideo *video,
                              gchar **mime,
                              gfloat *framerate,
                              gint *width,
                              gint *height)
{
  return grl_media_video_get_url_data_nth (video, 0, mime, framerate, width, height);
}

/**
 * grl_media_video_get_url_data_nth:
 * @video: the media instance
 * @index: element to retrieve
 * @mime: (out) (transfer none): the url mime-type, or %NULL to ignore
 * @framerate: the url framerate, or %NULL to ignore
 * @width: the url width, or %NULL to ignore
 * @height: the url height, or %NULL to ignore
 *
 * Returns: the n-th video's url, as well as its mime-type, framerate, width and
 * height.
 **/
const gchar *
grl_media_video_get_url_data_nth (GrlMediaVideo *video,
                                  guint index,
                                  gchar **mime,
                                  gfloat *framerate,
                                  gint *width,
                                  gint *height)
{
  GrlProperty *prop =
    grl_data_get_property (GRL_DATA (video), GRL_METADATA_KEY_URL, index);

  if (!prop) {
    return NULL;
  }

  if (mime) {
    *mime = (gchar *) grl_property_get_string (prop, GRL_METADATA_KEY_MIME);
  }

  if (framerate) {
    *framerate = grl_property_get_float (prop, GRL_METADATA_KEY_FRAMERATE);
  }

  if (width) {
    *width = grl_property_get_int (prop, GRL_METADATA_KEY_WIDTH);
  }

  if (height) {
    *height = grl_property_get_int (prop, GRL_METADATA_KEY_HEIGHT);
  }

  return grl_property_get_string (prop, GRL_METADATA_KEY_URL);
}
