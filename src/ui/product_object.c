#include "product_object.h"

struct _ProductObject {
    GObject parent;
    Product data;
};

G_DEFINE_TYPE(ProductObject, product_object, G_TYPE_OBJECT)

static void product_object_class_init(ProductObjectClass *klass) {
    (void)klass;
}

static void product_object_init(ProductObject *self) {
    (void)self;
}

ProductObject *product_object_new(const Product *p) {
    ProductObject *obj = g_object_new(PRODUCT_TYPE_OBJECT, NULL);
    obj->data = *p;
    return obj;
}

const Product *product_object_get_product(ProductObject *obj) {
    return &obj->data;
}
