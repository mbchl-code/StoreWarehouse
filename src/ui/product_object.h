#ifndef PRODUCT_OBJECT_H
#define PRODUCT_OBJECT_H

#include <glib-object.h>
#include "product.h"

#define PRODUCT_TYPE_OBJECT (product_object_get_type())
G_DECLARE_FINAL_TYPE(ProductObject, product_object, PRODUCT, OBJECT, GObject)

ProductObject  *product_object_new(const Product *p);
const Product  *product_object_get_product(ProductObject *obj);

#endif
