// Not for public distribution
//=======================================================================
//
// Contains the utility functions for reading and creating gdal_raster 
// TODO roll out the moving_window namespace to all files
// TODO rename this file to something more specific

#ifndef UTILITY_H_AHZ
#define UTILITY_H_AHZ

#include <gdal.h>
#include <gdal_priv.h>

#include <boost/filesystem.hpp>

#include <iostream>
//! \brief the namespace containing the moving window library
namespace moving_window {

  //! \brief Type trait for the GDALData type associated with T
  //! Overloads for int, unsigned int, short, unsigned short,unsigned char
  //! float and double are provided. 
  //! \tparam T needs to be overloaded by the user.
  template<typename T> struct native_gdal_data_type
  {
    static const GDALDataType type = GDT_Unknown;
  };

  // \cond 
  // exclude from doxygen
  template<> struct native_gdal_data_type<int>
  {
    static const GDALDataType type = GDT_Int32;
  };

  template<> 
  struct native_gdal_data_type<unsigned int>
  {
    static const GDALDataType type = GDT_UInt32;
  };

  template<> struct native_gdal_data_type<short>
  {
    static const GDALDataType type = GDT_Int16;
  };

  template<> struct native_gdal_data_type<unsigned short>
  {
    static const GDALDataType type = GDT_UInt16;
  };

  template<>
  struct native_gdal_data_type<unsigned char>
  {
    static const GDALDataType type = GDT_Byte;
  };

  template<>
  struct native_gdal_data_type<float>
  {
    static const GDALDataType type = GDT_Float32;
  };

  template<>
  struct native_gdal_data_type<double>
  {
    static const GDALDataType type = GDT_Float64;
  };
  // \endcond

 namespace detail {

    GDALDataset* create_standard_gdaldataset(
      const boost::filesystem::path& path, int rows, int cols
      , GDALDataType datatype, int nBands = 1)
    {
      GDALAllRegister();

      GDALDriverManager* m = GetGDALDriverManager();
      GDALDriver* driver = m->GetDriverByName("GTiff");

      char **papszOptions = NULL;
      papszOptions = CSLSetNameValue(papszOptions, "BLOCKXSIZE", "256");
      papszOptions = CSLSetNameValue(papszOptions, "BLOCKYSIZE", "256");
      papszOptions = CSLSetNameValue(papszOptions, "TILED", "YES");
      papszOptions = CSLSetNameValue(papszOptions, "COMPRESS", "DEFLATE");

      GDALDataset* dataset = driver->Create(path.string().c_str(), cols, rows
        , nBands, datatype, papszOptions);
      return dataset;
    }

    GDALDataset* create_standard_gdaldataset_from_model(
      const boost::filesystem::path& path, const GDALDataset* model
      , GDALDataType datatype, int nBands = 1)
    {
      int rows = const_cast<GDALDataset*>(model)->GetRasterYSize();
      int cols = const_cast<GDALDataset*>(model)->GetRasterXSize();

      GDALDataset* dataset = create_standard_gdaldataset(path, rows, cols
        , datatype, nBands);

      if (dataset == NULL) return NULL;

      double gt_data[6];
      double* geotransform = gt_data;
      CPLErr error_status 
        = const_cast<GDALDataset*>(model)->GetGeoTransform(geotransform);
      dataset->SetGeoTransform(geotransform);
      dataset->SetProjection(
        const_cast<GDALDataset*>(model)->GetProjectionRef());
      return dataset;
    }

    boost::filesystem::path get_temp_tiff_path()
    {
      boost::filesystem::path temp_path 
        = boost::filesystem::temp_directory_path();
      boost::filesystem::path unique_temp_path_model 
        = temp_path /= "%%%%-%%%%-%%%%-%%%%.tif";
      return unique_path(unique_temp_path_model);
    }
  }
  
  //! \brief open a raster file from disk.
  //! \param path the location of the file.
  //! \param access the type of access required, either GAReadOnly or GAUpdate
  //! \param band the raster band that will be accessed 
  template<typename T>
  gdal_raster<T> open_gdal_raster(const boost::filesystem::path& path
    , GDALAccess access, int band = 1)
  {
    return gdal_raster<T>(path, access, band);
  }

  //! \brief create a gdal_raster file on disk using standard setting 
  //! that make it efficient for use in moving window library.
  //! \tparam the type for pixel elements
  //! \param path the location for the file
  //! \param rows the row dimension of the file
  //! \param cols the column dimension of the file
  //! \param datatype the GDALDataType used for storage. By default it is
  //! derived from T
  template<typename T>
  gdal_raster<T> create_gdal_raster(const boost::filesystem::path& path
    , int rows, int cols, GDALDataType datatype = native_gdal_data_type<T>::type)
  {
    int nBands = 1;
    GDALDataset* dataset = detail::create_standard_gdaldataset(path, rows
      , cols, datatype, nBands);
    if (dataset == nullptr)
    {
      std::cout << "Could not create raster file: " << path << std::endl;
      BOOST_THROW_EXCEPTION(creating_a_raster_failed{});
    }
    gdal_raster<T> raster(dataset, path, nBands);
    return raster;
  }

  //! \brief create a gdal_raster using dimensions and projection information from another
  //! \tparam the type for pixel elements.
  //! \param path the location for the file
  //! \param model the dimensions and projection information of this gdal_raster will be copied
  //! \param datatype the GDALDataType used for storage. By default it is derived from T
  template<typename T, typename U>
  gdal_raster<T> create_gdal_raster_from_model(
    const boost::filesystem::path& path, const gdal_raster<U>& model, GDALDataType datatype = native_gdal_data_type<T>::type)
  {
    int nBands = 1;
    GDALDataset* dataset = detail::create_standard_gdaldataset_from_model(path, model.get_gdal_dataset(), datatype, nBands);
    if (dataset == nullptr)
    {
      std::cout << "Could not create raster file: " << path << std::endl;
      BOOST_THROW_EXCEPTION(creating_a_raster_failed{});
    }
    gdal_raster<T> raster(dataset, path, nBands);
    return raster;
  }

  //! \brief create a temporary gdal_raster
  //! The data file will be deleted as the gdal_raster is destructed.
  //! \tparam the type for pixel elements
  //! \param rows the row dimension of the file
  //! \param cols the column dimension of the file
  //! \param datatype the GDALDataType used for storage. By default it is derived from T
  template<typename T>
  gdal_raster<T> create_temp_gdal_raster(int rows, int cols, GDALDataType datatype = native_gdal_data_type<T>::type)
  {
    boost::filesystem::path path = detail::get_temp_tiff_path();
    gdal_raster<T> raster = create_gdal_raster<T>(path, rows, cols, datatype);
    raster.set_delete_on_close(true);
    return raster;
  }

  //! \brief create a temporary gdal_raster using dimensions and projection information from another
  //! The data file will be deleted as the gdal_raster is destructed.
  //! \tparam the type for pixel elements
  //! \param model the dimensions and projection information of this gdal_raster will be copied
  //! \param datatype the GDALDataType used for storage. By default it is derived from T
  template<typename T, typename U>
  gdal_raster<T> create_temp_gdal_raster_from_model(const gdal_raster<U>& model, GDALDataType datatype = native_gdal_data_type<T>::type)
  {
    boost::filesystem::path path = get_temp_tiff_path();
    gdal_raster<T> raster = create_gdal_raster_from_model<T>(path, model, datatype);
    raster.set_delete_on_close(true);
    return raster;
  }
}
using namespace moving_window;
#endif