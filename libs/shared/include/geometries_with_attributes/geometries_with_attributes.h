#pragma once
#include "defs.h"

namespace LxGeo
{
	namespace GeometryFactoryShared
	{
		template <typename T>
		class Geometries_with_attributes
		{
		public:
			Geometries_with_attributes(const T& _definition);
			~Geometries_with_attributes() { }

			void set_definition(const T& _definition) { definition = _definition; }
			const T& get_definition() const { return definition; }
			T& get_definition() { return definition; }

			void set_double_attribute(const std::string& name, const double& val) { double_attributes[name] = val; }
			double get_double_attribute(const std::string& name) const;

			void set_int_attribute(const std::string& name, const int& val) { int_attributes[name] = val; }
			int get_int_attribute(const std::string& name) const;

			void set_string_attribute(const std::string& name, const std::string& val) { string_attributes[name] = val; }
			std::string get_string_attribute(const std::string& name) const;

			void get_list_of_double_attributes(std::list<std::string>& A) const;
			void get_list_of_int_attributes(std::list<std::string>& A) const;
			void get_list_of_string_attributes(std::list<std::string>& A) const;

		protected:
			T definition;
			std::map<std::string, double> double_attributes;
			std::map<std::string, int> int_attributes;
			std::map<std::string, std::string> string_attributes;
		};

		template <typename geometry_type>
		std::vector<Geometries_with_attributes<geometry_type>> transform_to_geom_with_attr(std::vector<geometry_type>& in_geoms) {

			std::vector<Geometries_with_attributes<geometry_type>> geoms_with_attrs; geoms_with_attrs.reserve(in_geoms.size());

			std::transform(in_geoms.begin(), in_geoms.end(), std::back_inserter(geoms_with_attrs),
				[](geometry_type& in_geom)->Geometries_with_attributes<geometry_type> {Geometries_with_attributes<geometry_type> out_geom_with_attr(in_geom);
			return out_geom_with_attr; });

			return geoms_with_attrs;
		}
	}
}

#include "geometries_with_attributes.tpp"