#include "a4/object_store.h"

ObjectBackStore::ObjectBackStore() {};

ObjectBackStore::~ObjectBackStore() {};

ObjectStore ObjectBackStore::store() { return ObjectStore(&hl, this); };
