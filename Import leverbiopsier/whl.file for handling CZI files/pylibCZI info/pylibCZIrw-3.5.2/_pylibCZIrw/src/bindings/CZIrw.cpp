#pragma once

#include "../api/CZIreadAPI.h"
#include "../api/CZIwriteAPI.h"
#include "../api/PImage.h"
#include "PbHelper.h"

#include <Python.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/complex.h>
#include <pybind11/functional.h>
#include <pybind11/chrono.h>
#include <pybind11/options.h>

namespace py = pybind11;

PYBIND11_MODULE(_pylibCZIrw, m)
{
    py::class_<CZIreadAPI>(m, "czi_reader", py::module_local())
        .def(py::init<const std::wstring&>())
        .def("close", &CZIreadAPI::close)
        .def("GetXmlMetadata", &CZIreadAPI::GetXmlMetadata)
        .def("GetSubBlockStats", &CZIreadAPI::GetSubBlockStats)
        .def("GetDimensionSize", &CZIreadAPI::GetDimensionSize)
        .def("GetChannelPixelType", &CZIreadAPI::GetChannelPixelType)
        .def("GetSingleChannelScalingTileAccessorData", &CZIreadAPI::GetSingleChannelScalingTileAccessorData);

    py::class_<CZIwriteAPI>(m, "czi_writer", py::module_local())
        .def(py::init<const std::wstring&, const std::string&>())
        .def(py::init<const std::wstring&>())
        .def("close", &CZIwriteAPI::close)
        .def("WriteMetadata", &CZIwriteAPI::WriteMetadata)
        .def("AddTile", &CZIwriteAPI::AddTile)
        .def("AddTileEx", &CZIwriteAPI::AddTileEx);

    py::class_<PImage>(m, "PImage", py::buffer_protocol(), py::module_local())

        .def(py::init([](py::buffer b, libCZI::PixelType pixel_type) {

        /* Request a buffer descriptor from Python */
        py::buffer_info info = b.request();

        /* Some sanity checks ... */
        if (info.ndim != 3)
            throw std::runtime_error("Incompatible buffer dimension!");

        /* Convert Python Buffer to BitmapData */
        auto BitmapPtr = PbHelper::BufferToBitmap(b, pixel_type);

        std::unique_ptr<PImage> BitmapUptr(new PImage(BitmapPtr));
        return BitmapUptr;
            }))

        .def_buffer([](PImage& m) -> py::buffer_info {
                return py::buffer_info(
                    m.get_data(),                               // Pointer to buffer 
                    m.get_itemsize(),                           // Size of one scalar
                    PbHelper::get_format(m.get_pixelType()),    // Python struct-style format descriptor
                    m.get_ndim(),                               // Number of dimensions
                    m.get_shape(),                              // Buffer dimensions
                    {                                           // Strides (in bytes) for each index
                        m.get_stride(),
                        m.get_shape()[m.get_ndim() - 1] * m.get_itemsize(),
                        m.get_itemsize()
                    }
                );
            });

            py::class_<libCZI::SubBlockStatistics>(m, "SubBlockStatistics", py::module_local())
                .def(py::init<>())
                .def_readonly("subBlockCount", &libCZI::SubBlockStatistics::subBlockCount)
                .def_readonly("minMindex", &libCZI::SubBlockStatistics::minMindex)
                .def_readonly("maxMindex", &libCZI::SubBlockStatistics::maxMindex)
                .def_readonly("boundingBox", &libCZI::SubBlockStatistics::boundingBox)
                .def_readonly("boundingBoxLayer0Only", &libCZI::SubBlockStatistics::boundingBoxLayer0Only)
                //.def_readonly("dimBounds", &libCZI::SubBlockStatistics::dimBounds)
                .def_readonly("sceneBoundingBoxes", &libCZI::SubBlockStatistics::sceneBoundingBoxes);

            py::class_<libCZI::BoundingBoxes>(m, "BoundingBoxes", py::module_local())
                .def(py::init<>())
                .def_readwrite("boundingBox", &libCZI::BoundingBoxes::boundingBox)
                .def_readwrite("boundingBoxLayer0", &libCZI::BoundingBoxes::boundingBoxLayer0);


            py::class_<libCZI::RgbFloatColor>(m, "RgbFloatColor", py::module_local())
                .def(py::init<>())
                .def_readwrite("b", &libCZI::RgbFloatColor::b)
                .def_readwrite("g", &libCZI::RgbFloatColor::g)
                .def_readwrite("r", &libCZI::RgbFloatColor::r);

            py::class_<libCZI::IntRect>(m, "IntRect", py::module_local())
                .def(py::init<>())
                .def_readwrite("x", &libCZI::IntRect::x)
                .def_readwrite("y", &libCZI::IntRect::y)
                .def_readwrite("w", &libCZI::IntRect::w)
                .def_readwrite("h", &libCZI::IntRect::h);

            py::enum_<libCZI::DimensionIndex>(m, "DimensionIndex", py::module_local())
                .value("Z", libCZI::DimensionIndex::Z)
                .value("C", libCZI::DimensionIndex::C)
                .value("T", libCZI::DimensionIndex::T)
                .value("R", libCZI::DimensionIndex::R)
                .value("S", libCZI::DimensionIndex::S)
                .value("I", libCZI::DimensionIndex::I)
                .value("H", libCZI::DimensionIndex::H)
                .value("V", libCZI::DimensionIndex::V)
                .value("B", libCZI::DimensionIndex::B)
                .export_values();

            py::enum_<libCZI::PixelType>(m, "PixelType", py::module_local())
                .value("Gray8", libCZI::PixelType::Gray8)
                .value("Gray16", libCZI::PixelType::Gray16)
                .value("Gray32", libCZI::PixelType::Gray32)
                .value("Bgr24", libCZI::PixelType::Bgr24)
                .value("Bgr48", libCZI::PixelType::Bgr48)
                .value("Gray32Float", libCZI::PixelType::Gray32Float)
                .value("Bgr96Float", libCZI::PixelType::Bgr96Float)
                .value("Invalid", libCZI::PixelType::Invalid)
                .export_values();

            py::class_<libCZI::CustomValueVariant>(m, "CustomValueVariant", py::module_local())
                .def(py::init<>())
                .def_property("int32Value", &libCZI::CustomValueVariant::GetAsInt32OrThrow, &libCZI::CustomValueVariant::SetInt32)
                .def_property("floatValue", &libCZI::CustomValueVariant::GetAsFloatOrThrow, &libCZI::CustomValueVariant::SetFloat)
                .def_property("doubleValue", &libCZI::CustomValueVariant::GetAsDoubleOrThrow, &libCZI::CustomValueVariant::SetDouble)
                .def_property("boolValue", &libCZI::CustomValueVariant::GetAsBoolOrThrow, &libCZI::CustomValueVariant::SetBool)
                .def_property("stringValue", &libCZI::CustomValueVariant::GetAsStringOrThrow, &libCZI::CustomValueVariant::SetString);

            py::enum_<TintingModeEnum>(m, "TintingModeEnum", py::module_local())
                .value("None", TintingModeEnum::None)
                .value("Color", TintingModeEnum::Color)
                .value("LookUpTableExplicit", TintingModeEnum::LookUpTableExplicit)
                .value("LookUpTableWellKnown", TintingModeEnum::LookUpTableWellKnown)
                .export_values();

            py::class_<libCZI::Rgb8Color>(m, "Rgb8Color", py::module_local())
                .def(py::init<>())
                .def_readwrite("r", &libCZI::Rgb8Color::r)
                .def_readwrite("g", &libCZI::Rgb8Color::g)
                .def_readwrite("b", &libCZI::Rgb8Color::b);

            py::class_<ChannelDisplaySettingsStruct>(m, "ChannelDisplaySettingsStruct", py::module_local())
                .def(py::init<>())
                .def_readwrite("isEnabled", &ChannelDisplaySettingsStruct::isEnabled)
                .def_readwrite("tintingMode", &ChannelDisplaySettingsStruct::tintingMode)
                .def_readwrite("tintingColor", &ChannelDisplaySettingsStruct::tintingColor)
                .def_readwrite("blackPoint", &ChannelDisplaySettingsStruct::blackPoint)
                .def_readwrite("whitePoint", &ChannelDisplaySettingsStruct::whitePoint)
                .def("Clear", &ChannelDisplaySettingsStruct::Clear);
}