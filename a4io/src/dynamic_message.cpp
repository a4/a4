#include "dynamic_message.h"

using google::protobuf::Message;
using google::protobuf::FieldDescriptor;
using google::protobuf::Reflection;

void add_fields(const ConstDynamicField& f1, const ConstDynamicField& f2, DynamicField& merged) {
    if (f1.repeated()) {
        if (f1.size() != f2.size()) 
            FATAL("Trying to add arrays of different sizes in ", f1.name());
        for (int i = 0; i < f1.size(); i++) {
            merged.add(f1.value(i) + f2.value(i));
        }
    } else {
        merged.set(f1.value() + f2.value());
    }
}

void multiply_fields(const ConstDynamicField& f1, const ConstDynamicField& f2, DynamicField& merged) {
    if (f1.repeated()) {
        if (f1.size() != f2.size()) 
            FATAL("Trying to add arrays of different sizes in ", f1.name());
        for (int i = 0; i < f1.size(); i++) {
            merged.add(f1.value(i) * f2.value(i));
        }
    } else {
        merged.set(f1.value() * f2.value());
    }
}

void append_fields(const ConstDynamicField& f1, const ConstDynamicField& f2, DynamicField& merged, bool make_unique) {
    if (!f1.repeated()) 
        FATAL("MERGE_UNION/APPEND is not applicable to non-repeated field ", f1.name());
        
    std::unordered_set<FieldContent> items;
    for (int i = 0; i < f1.size(); i++) {
        FieldContent fc = f1.value(i);
        if (make_unique) {
            if (items.find(fc) == items.end()) 
                merged.add(fc);
                
            items.insert(fc);
        } else {
            merged.add(fc);
        }
    }
    for (int i = 0; i < f2.size(); i++) {
        FieldContent fc = f2.value(i);
        if (make_unique) {
            if (items.find(fc) == items.end()) 
                merged.add(fc);
                
            items.insert(fc);
        } else {
            merged.add(fc);
        }
    }
}

void inplace_add_fields(DynamicField& merged, ConstDynamicField& f2) {
    if (merged.repeated()) {
        throw a4::Fatal("Not implemented: inplace_add_fields with repeated field");
    } else {
        merged.set(merged.value() + f2.value());
    }
}

void inplace_multiply_fields(DynamicField& merged, ConstDynamicField& f2) {
    if (merged.repeated()) {
        throw a4::Fatal("Not implemented: inplace_multiply_fields with repeated field");
    } else {
        merged.set(merged.value() * f2.value());
    }
}

void inplace_append_fields(DynamicField& merged, ConstDynamicField& f2) {
    if (!merged.repeated())
        throw a4::Fatal("MERGE_UNION/APPEND is not applicable to non-repeated field ", merged.name());
        
    for (int i = 0; i < f2.size(); i++)
        merged.add(f2.value(i));
}

