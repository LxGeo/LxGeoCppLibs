#include "defs.h"
#include "defs_cgal.h"
#include "defs_boost.h"
#include "defs_common.h"
#include "defs_opencv.h"
#include "export_shared.h"

namespace LxGeo
{
	namespace GeometryFactoryShared
	{

		int mod(int a, int b) {
			int c = a % b;
			return (c < 0) ? c + b : c;
		}

		std::string random_string(const size_t& length, const std::string& prefix, const std::string& suffix)
		{
			std::string str("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");

			std::random_device rd;
			std::mt19937 generator(rd());

			std::shuffle(str.begin(), str.end(), generator);

			return prefix + str.substr(0, length) + suffix;
		}

		Inexact_Point_2 transform_B2C_Point(const Boost_Point_2& boost_point) {
			return Inexact_Point_2(bg::get<0>(boost_point), bg::get<1>(boost_point));
		};

		Boost_Point_2 transform_C2B_Point(const Inexact_Point_2& cgal_point) {
			return Boost_Point_2(cgal_point.x(), cgal_point.y());
		};

		Boost_Point_2 transform_C2B_Point(const Point_2& cgal_point) {
			return Boost_Point_2(CGAL::to_double(cgal_point.x()), CGAL::to_double(cgal_point.y()));
		};

		void container_transform_B2C_Points(std::vector<Boost_Point_2>& input_container, std::vector<Inexact_Point_2>& output_container) {
			assert(output_container.empty());
			output_container.reserve(input_container.size());

			for (Boost_Point_2 c_point : input_container) {
				Inexact_Point_2 cgal_point = transform_B2C_Point(c_point);
				output_container.push_back(cgal_point);
			}

		};

		void container_transform_OGRRING2vector_Points(OGRLinearRing* input_ring, std::vector<Inexact_Point_2>& output_vector) {

			output_vector.reserve(input_ring->getNumPoints());

			for (size_t c_point_idx = 0; c_point_idx < input_ring->getNumPoints(); ++c_point_idx) {
				output_vector.push_back(Inexact_Point_2(input_ring->getX(c_point_idx), input_ring->getY(c_point_idx)));
			}
		}

		std::vector<Inexact_Point_2> container_transform_OGRRING2vector_Points(OGRLinearRing* input_ring) {

			std::vector<Inexact_Point_2> aux_vector;
			container_transform_OGRRING2vector_Points(input_ring, aux_vector);
			return aux_vector;
		}

		OGRLinearRing container_transform_vector_Points2OGRRING(const std::vector<Inexact_Point_2>& input_vector) {

			OGRLinearRing aux_ogr_ring;
			for (const Inexact_Point_2& c_point : input_vector) {
				OGRPoint c_ogr_point(c_point.x(), c_point.y());
				aux_ogr_ring.addPoint(&c_ogr_point);
			}
			return aux_ogr_ring;
		}

		Boost_Polygon_2 transform_OGR2B_Polygon(OGRPolygon* ogr_polygon) {
			Boost_Polygon_2 aux_boost_polygon;

			// fill exterior
			OGRLinearRing* exterior_ring = ogr_polygon->getExteriorRing();
			for (size_t v_idx = 0; v_idx < exterior_ring->getNumPoints(); ++v_idx) {
				OGRPoint pt;
				exterior_ring->getPoint(v_idx, &pt);
				double x = pt.getX(), y = pt.getY();
				aux_boost_polygon.outer().push_back(Boost_Point_2(x, y));
			}
			// fill interior
			for (size_t int_ring_idx = 0; int_ring_idx < ogr_polygon->getNumInteriorRings(); ++int_ring_idx) {
				OGRLinearRing* c_int_ring = ogr_polygon->getInteriorRing(int_ring_idx);
				Boost_Ring_2 R;
				int ring_size = c_int_ring->getNumPoints();
				R.reserve(ring_size);
				for (int u = 0; u < ring_size; ++u) { //-1
					OGRPoint pt;
					c_int_ring->getPoint(u, &pt);
					double x = pt.getX(), y = pt.getY();
					//CT->Transform(1, &x, &y);
					R.push_back(Boost_Point_2(x, y));
				}
				aux_boost_polygon.inners().push_back(R);
			}
			return aux_boost_polygon;

		}

		Boost_LineString_2 transform_OGR2B_Linestring(OGRLineString* ogr_linestring) {
			Boost_LineString_2 aux_boost_linestring;
			for (size_t v_idx = 0; v_idx < ogr_linestring->getNumPoints(); ++v_idx) {
				OGRPoint pt;
				ogr_linestring->getPoint(v_idx, &pt);
				aux_boost_linestring.push_back(transform_OGR2B_Point(&pt));
			}
			return aux_boost_linestring;
		}

		Boost_Point_2 transform_OGR2B_Point(OGRPoint* ogr_point){
			return Boost_Point_2(ogr_point->getX(), ogr_point->getY());
		}

		double angle3p(const Boost_Point_2& p_m, const Boost_Point_2& p_0, const Boost_Point_2& p_1) {

			double m0x = p_0.get<0>() - p_m.get<0>(), m0y = p_0.get<1>() - p_m.get<1>(),
				m1x = p_1.get<0>() - p_m.get<0>(), m1y = p_1.get<1>() - p_m.get<1>();

			double angleRadians = atan2(m0x * m1x + m0y * m1y, m0y * m1x - m0x * m1y);
			angleRadians = std::fmod(angleRadians + M_PI, M_PI);
			return angleRadians;
		}

		Boost_Box_2 box_buffer(Boost_Box_2& in_box, double buff) {
			Boost_Box_2 out_box;
			out_box.min_corner().set<0>(in_box.min_corner().get<0>() - buff);
			out_box.min_corner().set<1>(in_box.min_corner().get<1>() - buff);
			out_box.max_corner().set<0>(in_box.max_corner().get<0>() + buff);
			out_box.max_corner().set<1>(in_box.max_corner().get<1>() + buff);
			return out_box;
		}

		OGREnvelope box_buffer(OGREnvelope& in_box, double buff) {
			OGREnvelope out_box(in_box);
			out_box.MinX -= buff;
			out_box.MinY -= buff;
			out_box.MaxX += buff;
			out_box.MaxY += buff;
			return out_box;
		}

		OGRPoint transform_B2OGR_Point(const Boost_Point_2& in_point) {
			return OGRPoint(in_point.get<0>(), in_point.get<1>());
		}

		OGRLinearRing transform_B2OGR_LinearRing(const Boost_Ring_2& in_ring) {
			OGRLinearRing ogr_linering;
			for (const Boost_Point_2& c_pt : in_ring) {
				OGRPoint c_ogr_point = transform_B2OGR_Point(c_pt);
				ogr_linering.addPoint(&c_ogr_point);
			}
			return ogr_linering;
		}

		OGRLinearRing transform_B2OGR_Ring(const Boost_Ring_2& in_ring) {
			OGRLinearRing ogr_ring;
			for (const Boost_Point_2& c_pt : in_ring) {
				OGRPoint c_ogr_point = transform_B2OGR_Point(c_pt);
				ogr_ring.addPoint(&c_ogr_point);
			}
			return ogr_ring;
		}

		OGRPolygon transform_B2OGR_Polygon(const Boost_Polygon_2& in_polygon) {

			OGRPolygon out_polygon;

			if (in_polygon.outer().empty()) return out_polygon;

			OGRLinearRing ogr_ext_ring = transform_B2OGR_Ring(in_polygon.outer());

			std::list<OGRLinearRing> ogr_int_rings;
			for (auto& c_int_ring : in_polygon.inners()) {
				ogr_int_rings.push_back(transform_B2OGR_Ring(c_int_ring));
			}

			out_polygon.addRing(&ogr_ext_ring);
			for (OGRLinearRing ring : ogr_int_rings) out_polygon.addRing(&ring);
			return out_polygon;
		}

		OGRLineString transform_B2OGR_LineString(const Boost_LineString_2& in_linestring) {
			OGRLineString out_linestring;
			for (auto& c_pt : in_linestring) {
				OGRPoint c_ogr_point = transform_B2OGR_Point(c_pt);
				out_linestring.addPoint(&c_ogr_point);
			}
			return out_linestring;
		}

		OGREnvelope transform_B2OGR_Envelope(const Boost_Box_2& in_envelope) {
			OGREnvelope out_envelope;
			out_envelope.MinX = in_envelope.min_corner().get<0>();
			out_envelope.MinY = in_envelope.min_corner().get<1>();
			out_envelope.MaxX = in_envelope.max_corner().get<0>();
			out_envelope.MaxY = in_envelope.max_corner().get<1>();
			return out_envelope;
		}

		Boost_Box_2 transform_OGR2B_Envelope(const OGREnvelope& in_envelope) {
			return { {in_envelope.MinX, in_envelope.MinY}, {in_envelope.MaxX, in_envelope.MaxY} };
		}

		OGRPolygon envelopeToPolygon(const OGREnvelope& in_envelope) {
			OGRPolygon output_polygon;
			OGRLinearRing ring;
			ring.addPoint(in_envelope.MinX, in_envelope.MinY);
			ring.addPoint(in_envelope.MaxX, in_envelope.MinY);
			ring.addPoint(in_envelope.MaxX, in_envelope.MaxY);
			ring.addPoint(in_envelope.MinX, in_envelope.MaxY);
			ring.closeRings();
			output_polygon.addRing(&ring);
			return output_polygon;
		}

		void transform_G2CV_affine(const double in_geotransform[6], cv::Mat& out_affine_matrix) {
			out_affine_matrix = (cv::Mat_<double>(2, 3) <<
				in_geotransform[1], in_geotransform[2], in_geotransform[0],
				in_geotransform[4], in_geotransform[5], in_geotransform[3]);

		}

		void transform_CV2G_affine(const cv::Mat& affine_matrix, double outtransform[6]) {
			outtransform[0] = affine_matrix.at<double>(0, 2);
			outtransform[1] = affine_matrix.at<double>(0, 0);
			outtransform[2] = affine_matrix.at<double>(0, 1);
			outtransform[3] = affine_matrix.at<double>(1, 2);
			outtransform[4] = affine_matrix.at<double>(1, 0);
			outtransform[5] = affine_matrix.at<double>(1, 1);
		};

		cv::Mat multiply_affine_matrices(const cv::Mat& m1, const cv::Mat& m2) {
			assert (m1.size() == m2.size() && m1.size().width == 3 && m1.size().height == 2);

			cv::Mat homography1 = cv::Mat::eye(3, 3, CV_64FC1);
			cv::Mat homography2 = cv::Mat::eye(3, 3, CV_64FC1);
			
			cv::Mat aux1 = homography1.colRange(0, 3).rowRange(0, 2);
			cv::Mat aux2 = homography2.colRange(0, 3).rowRange(0, 2);
			m1.copyTo(aux1);
			m2.copyTo(aux2);

			cv::Mat product = homography1 * homography2;

			return product.colRange(0, 3).rowRange(0, 2);
		}

		std::shared_ptr<GDALDataset> load_gdal_dataset_shared_ptr(const std::string& raster_file_path) {
			bool file_exists = boost::filesystem::exists(raster_file_path);
			if (!file_exists) {
				auto err_msg = "File not found at path " + raster_file_path;
				throw std::runtime_error(err_msg.c_str());
			}
			GDALDataset* dst = (GDALDataset*)GDALOpen(raster_file_path.c_str(), GA_ReadOnly);
			if (dst == NULL) {
				auto err_msg = "Unable to open raster dataset in read mode from file " + raster_file_path;
				throw std::runtime_error(err_msg.c_str());
			}
			return std::shared_ptr<GDALDataset>(dst, GDALClose);
		};

		std::shared_ptr<GDALDataset> load_gdal_vector_dataset_shared_ptr(const std::string& vector_file_path, int extra_flags ) {
			bool file_exists = boost::filesystem::exists(vector_file_path);
			if (!file_exists) {
				auto err_msg = "File not found at path " + vector_file_path;
				throw std::runtime_error(err_msg.c_str());
			}
			GDALDataset* dst = (GDALDataset*)GDALOpenEx(vector_file_path.c_str(), GDAL_OF_VECTOR | extra_flags, NULL, NULL, NULL);
			if (dst == NULL) {
				auto err_msg = "Unable to open vector dataset in read mode from file " + vector_file_path;
				throw std::runtime_error(err_msg.c_str());
			}
			return std::shared_ptr<GDALDataset>(dst, GDALClose);
		};
		

	}
}

int mod(int a, int b)
{
	return 0;
}
