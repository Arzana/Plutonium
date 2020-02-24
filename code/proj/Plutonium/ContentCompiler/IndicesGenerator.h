#pragma once
#include "PumData.h"
#include <Streams/BinaryWriter.h>
#include <mutex>

void GenerateIndices(pum_mesh &mesh, const void *vertices, Pu::BinaryWriter &writer, std::mutex *lock);