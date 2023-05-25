#include "satellites/imd.h"


namespace LxGeo
{
	namespace GeometryFactoryShared
	{
		IMetaData::IMetaData(std::string& to_parse_filepath) {
			filepath = boost::filesystem::path(to_parse_filepath);
			std::string extension = boost::filesystem::extension(filepath);
			std::transform(extension.begin(), extension.end(), extension.begin(),
				[](unsigned char c) { return std::tolower(c); });

			setStrategy(extension);
			strat->parse(this);
		}


		void IMetaData::setStrategy(std::string& extension) {
			if (extension.compare(".imd") == 0)
				strat = std::move(std::make_unique<imdStrategy>());
			else if (extension.compare(".xml") == 0)
				strat = std::move(std::make_unique<xmlStrategy>());
			else
				throw std::runtime_error("Metadata file extension parser unvailable!");
		}


		void imdStrategy::parse(IMetaData* meta_obj) {
			std::ifstream t(meta_obj->filepath.string());
			std::stringstream buffer; buffer << t.rdbuf();
			const std::string file_content = buffer.str();

			std::regex numerical_pattern = std::regex("(\\w+) = ([0-9]+.[0-9]+)"); // std::regex("(?P<key>\w+) = (?P<value>\d+\.\d+)");
			std::regex str_pattern = std::regex("(\\w+) = \"([a-zA-Z0-9]+)\"");  //std::regex("(?P<key>\w+) = \"(? P<value>\S + )\"");


			std::smatch numerical_match, str_match;
			//std::regex_search(file_content.begin(), file_content.end(), numerical_match, numerical_pattern);
			//std::regex_search(file_content.begin(), file_content.end(), str_match, str_pattern);

			std::unordered_map<std::string, double> numerical_attributes;
			std::unordered_map<std::string, std::string> str_attributes;

			std::string search_text = file_content;
			while (std::regex_search(search_text, numerical_match, numerical_pattern)) {
				numerical_attributes[numerical_match[1].str()] = atof(numerical_match[2].str().c_str());
				search_text = numerical_match.suffix().str();
			}

			search_text = file_content;
			while (std::regex_search(search_text, str_match, str_pattern)) {
				str_attributes[str_match[1].str()] = numerical_match[2].str();
				search_text = str_match.suffix().str();
			}

			try {
				meta_obj->sunAzimuth = numerical_attributes["meanSunAz"];
				meta_obj->sunElevation = numerical_attributes["meanSunEl"];
				meta_obj->satAzimuth = numerical_attributes["meanSatAz"];
				meta_obj->satElevation = numerical_attributes["meanSatEl"];
				meta_obj->satelliteID = numerical_attributes["satId"];
			}
			catch (const std::exception& e) {
				std::cout << "Error loading attribute from imd file!" << std::endl;
				std::cout << e.what() << std::endl;
			};
		}

		void xmlStrategy::parse(IMetaData* meta_obj) {

			namespace pt = boost::property_tree;
			pt::ptree tree;
			pt::read_xml(meta_obj->filepath.string(), tree);
			meta_obj->satelliteID = tree.get<std::string>("Dimap_Document.Dataset_Sources.Source_Identification.Strip_Source.MISSION");

			auto mean_of_attributes = [&tree](const char* parent_name, const char* attribute_name) {
				std::list<double> values_list;
				auto k = tree.get_child(parent_name);
				BOOST_FOREACH(pt::ptree::value_type & v, tree.get_child(parent_name)) {
					values_list.push_back(v.second.get<double>(attribute_name));
				}
				double mean_of_angles = numcpp::statsFn::circular_mean(values_list, 0, 360);
				return mean_of_angles;
			};
			meta_obj->sunAzimuth = mean_of_attributes("Dimap_Document.Geometric_Data.Use_Area", "Solar_Incidences.SUN_AZIMUTH");
			meta_obj->sunElevation = mean_of_attributes("Dimap_Document.Geometric_Data.Use_Area", "Solar_Incidences.SUN_ELEVATION");

			double azimA = mean_of_attributes("Dimap_Document.Geometric_Data.Use_Area", "Acquisition_Angles.AZIMUTH_ANGLE");
			double IncAAccross = mean_of_attributes("Dimap_Document.Geometric_Data.Use_Area", "Acquisition_Angles.INCIDENCE_ANGLE_ACROSS_TRACK");
			double IncAAlong = mean_of_attributes("Dimap_Document.Geometric_Data.Use_Area", "Acquisition_Angles.INCIDENCE_ANGLE_ALONG_TRACK");
			double IncAngle = mean_of_attributes("Dimap_Document.Geometric_Data.Use_Area", "Acquisition_Angles.INCIDENCE_ANGLE");

			double IncAAccrossRAD = IncAAccross * M_PI / 180;
			double IncAAlongRAD = IncAAlong * M_PI / 180;

			double tanIncAAlongRAD = std::tan(IncAAlongRAD);
			double tanIncAAccrossRAD = std::tan(IncAAccrossRAD);
			double angle_atan2 = std::atan2(tanIncAAlongRAD, tanIncAAccrossRAD);
			double angle_atan2_deg = angle_atan2 * 180 / M_PI;
			double remainder = std::fmod(angle_atan2_deg + 360, 360);
			double meanSatAz = remainder + 90;
			/*double meanSatAz = std::fmod(
				(std::atan2(std::tan(IncAAlongRAD), std::tan(IncAAccrossRAD)) * 180 / M_PI)
				, 360
			) + 90;*/
			double temp_azi = std::fmod((azimA - (std::atan2(std::tan(IncAAlongRAD), std::tan(IncAAccrossRAD)) * 180 / M_PI)), 360);

			double finalAzi;
			if (std::abs<double>(azimA - meanSatAz) < 1.0)
				finalAzi = azimA;
			else {
				if (std::abs<double>(azimA - 90.0) < 1.0)
					finalAzi = meanSatAz + azimA - 90.0;
				else if (std::abs<double>(azimA - 180.0) < 1.0)
					finalAzi = meanSatAz + azimA - 180.0;
				else
					finalAzi = meanSatAz;
			}

			meta_obj->satAzimuth = finalAzi;
			meta_obj->satElevation = -IncAngle + 90;
		}

	}
}