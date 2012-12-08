#ifndef _A4_ROOT_COPY_CHAIN_H_
#define _A4_ROOT_COPY_CHAIN_H_

#include <google/protobuf/compiler/importer.h>
#include <google/protobuf/message.h>
#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/descriptor.h>
using google::protobuf::MessageFactory;
using google::protobuf::Descriptor;

#include <TChain.h>

#include <a4/types.h>
#include <a4/output_stream.h>

/// Copies `tree` into the `stream` using information taken from the compiled in
/// Event class.
void copy_chain(TChain& tree, shared<a4::io::OutputStream> stream,
    MessageFactory* dynamic_factory, const Descriptor* message_descriptor,
    Long64_t entries=-1, const uint32_t metadata_frequency=100000,
    const Long64_t initial_offset=0);
    
#endif
