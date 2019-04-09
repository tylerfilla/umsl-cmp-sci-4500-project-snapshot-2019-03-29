/*
 * Cozmonaut
 * Copyright (c) 2019 The Cozmonaut Contributors
 *
 * InsertLicenseText
 */

#include "common_image.h"
#include "image.h"

struct SF__CommonImage {
  SF__Image base;

  /** The backing image. */
  const faces::Image& mCom;

  explicit SF__CommonImage(const faces::Image& pCom);
};

SF__CommonImage::SF__CommonImage(const faces::Image& pCom)
    : base()
    , mCom(pCom) {
  base.getBackingImage = [](SFImage self) {
    return (void*) &((decltype(this)) self)->mCom;
  };
  base.getWidth = [](SFImage self) {
    return ((decltype(this)) self)->mCom.width;
  };
  base.getHeight = [](SFImage self) {
    return ((decltype(this)) self)->mCom.height;
  };
  base.getData = [](SFImage self) {
    return (void*) ((decltype(this)) self)->mCom.data.data();
  };
  base.getWidthStep = [](SFImage self) {
    return ((decltype(this)) self)->mCom.width * 3;
  };
}

int sfCommonImageCreate(SFCommonImage* image, const faces::Image& com) {
  *image = new SF__CommonImage(com);
  return 0;
}

int sfCommonImageDestroy(SFCommonImage image) {
  delete image;
  return 0;
}
