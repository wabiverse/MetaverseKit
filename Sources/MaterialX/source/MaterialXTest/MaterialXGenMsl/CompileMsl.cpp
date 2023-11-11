//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include "CompileMsl.h"
#include <Foundation/Foundation.hpp>

#include <streambuf>
#include <string>

#include <MaterialXGenShader/ShaderGenerator.h>

#include <Metal/Metal.hpp>
MTL::Device *device = nil;

void CompileMslShader(const char *pShaderFilePath, const char *pEntryFuncName) {
  NS::Error *error = nil;
  if (device == nil)
    device = MTL::CreateSystemDefaultDevice();

  NS::String *filepath = NS::String::string(pShaderFilePath, NS::UTF8StringEncoding);
  NS::String *shadersource = NS::String::string(filepath->fileSystemRepresentation(), NS::UTF8StringEncoding);
  if (error != nil) {
    throw MaterialX::ExceptionShaderGenError(
        "Cannot load file '" + std::string(pShaderFilePath) + "'.");
    return;
  }

  MTL::CompileOptions *options = MTL::CompileOptions::alloc()->init();
#if defined(MAC_OS_VERSION_11_0) || defined(IPHONE_OS_VERSION_14_0)
  options->setLanguageVersion(MTL::LanguageVersion2_3);
#else
  options->setLanguageVersion(MTL::LanguageVersion2_0);
#endif // defined(MAC_OS_VERSION_11_0) || defined(IPHONE_OS_VERSION_14_0)
  options->setFastMathEnabled(true);

  MTL::Library *library = device->newLibrary(shadersource, options, &error);
  if (library == nil) {
    throw MaterialX::ExceptionShaderGenError(
        "Failed to create library out of '" + std::string(pShaderFilePath) +
        "'." +
        std::string(error ? error->localizedDescription()->utf8String() : ""));
    return;
  }
}
