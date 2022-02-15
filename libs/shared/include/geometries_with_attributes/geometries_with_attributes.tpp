
namespace LxGeo 
{
	namespace GeometryFactoryShared
	{
		template <typename T>
		Geometries_with_attributes<T>::Geometries_with_attributes(const T & _definition)
		{
			set_definition(_definition);
		}

		template <typename T>
		int Geometries_with_attributes<T>::get_int_attribute(const std::string & name) const
		{
			auto it = int_attributes.find(name);
			if (it == int_attributes.cend()) {
				throw std::exception("Error : couldn't find attribute.");
			}

			return it->second;
		}

		template <typename T>
		double Geometries_with_attributes<T>::get_double_attribute(const std::string & name) const
		{
			auto it = double_attributes.find(name);
			if (it == double_attributes.cend()) {
				throw std::exception("Error : couldn't find attribute.");
			}

			return it->second;
		}

		template <typename T>
		std::string Geometries_with_attributes<T>::get_string_attribute(const std::string & name) const
		{
			auto it = string_attributes.find(name);
			if (it == string_attributes.cend()) {
				throw std::exception("Error : couldn't find attribute.");
			}

			return it->second;
		}

		template <typename T>
		void Geometries_with_attributes<T>::get_list_of_int_attributes(std::list<std::string> & A) const
		{
			for (auto it = int_attributes.cbegin(); it != int_attributes.cend(); ++it) {
				A.push_back(it->first);
			}
		}

		template <typename T>
		void Geometries_with_attributes<T>::get_list_of_double_attributes(std::list<std::string> & A) const
		{
			for (auto it = double_attributes.cbegin(); it != double_attributes.cend(); ++it) {
				A.push_back(it->first);
			}
		}

		template <typename T>
		void Geometries_with_attributes<T>::get_list_of_string_attributes(std::list<std::string> & A) const
		{
			for (auto it = string_attributes.cbegin(); it != string_attributes.cend(); ++it) {
				A.push_back(it->first);
			}
		}

	}
}