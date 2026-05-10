#include "product_object.h"

struct _ProductObject {
    GObject parent;
    Product data;
};

G_DEFINE_TYPE(ProductObject, product_object, G_TYPE_OBJECT)

// GObject class initialiser — no custom class members to set up
/*
 * Required by G_DEFINE_TYPE. All behaviour is inherited from GObject.
 */
static void product_object_class_init(ProductObjectClass *klass) {
    (void)klass;
}

// GObject instance initialiser — no per-instance setup required
/*
 * The Product data field is populated explicitly via product_object_new.
 */
static void product_object_init(ProductObject *self) {
    (void)self;
}

// Allocate a new ProductObject and copy the given product into it
/*
 * The returned object has a reference count of 1; the caller is
 * responsible for calling g_object_unref when done.
 */
ProductObject *product_object_new(const Product *p) {
    ProductObject *obj = g_object_new(PRODUCT_TYPE_OBJECT, NULL);
    obj->data = *p;
    return obj;
}

// Return a const pointer to the Product stored inside the object
/*
 * The pointer is valid for the lifetime of the ProductObject.
 * Do not free or modify the returned pointer directly.
 */
const Product *product_object_get_product(ProductObject *obj) {
    return &obj->data;
}
