#pragma once
#include "defs.h"
#include "defs_boost.h"
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <regex>
#include "numcpp/stats.h"
#include "export_shared.h"

namespace LxGeo
{
	namespace GeometryFactoryShared 
	{
		class ParsingStrategy;

		// IMetaData class
		class IMetaData {
		public:
			LX_GEO_FACTORY_SHARED_API IMetaData(std::string& to_parse_filepath);

			std::unique_ptr<ParsingStrategy> strat;
			boost::filesystem::path filepath;
			std::string satelliteID;
			double satAzimuth, satElevation, sunAzimuth, sunElevation;

			void setStrategy(std::string& extension);
		};

		class ParsingStrategy {
		public:
			virtual void parse(IMetaData* meta_obj) = 0;
			virtual ~ParsingStrategy() {}
		};

		class imdStrategy : public ParsingStrategy {
		public:
			LX_GEO_FACTORY_SHARED_API void parse(IMetaData* meta_obj) override;
		};

		// xmlStrategy class
		class xmlStrategy : public ParsingStrategy {
		public:
			LX_GEO_FACTORY_SHARED_API void parse(IMetaData* meta_obj) override;
		};		

	}
}