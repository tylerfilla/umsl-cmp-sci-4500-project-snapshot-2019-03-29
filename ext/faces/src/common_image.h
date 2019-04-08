/*
 * Cozmonaut
 * Copyright (c) 2019 The Cozmonaut Contributors
 *
 * InsertLicenseText
 */

#ifndef COMMON_IMAGE_H
#define COMMON_IMAGE_H

#include <faces/source.h>

/**
 * A spdyface image backed by a faces::Image.
 */
typedef struct SF__CommonImage* SFCommonImage;

/**
 * Create a spdyface faces::Image image.
 *
 * @param image The image destination
 * @param com The backing image
 * @return Zero on success, otherwise nonzero
 */
int sfCommonImageCreate(SFCommonImage* image, const faces::Image& com);

/**
 * Destroy a spdyface faces::Image image.
 *
 * @param image The image
 * @return Zero on success, otherwise nonzero
 */
int sfCommonImageDestroy(SFCommonImage image);

#endif // #ifndef COMMON_IMAGE_H
