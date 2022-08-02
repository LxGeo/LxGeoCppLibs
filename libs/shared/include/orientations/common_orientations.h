#pragma once
#include "defs.h"
#include "export_shared.h"

namespace LxGeo
{
	namespace GeometryFactoryShared
	{

		enum orientationSymbol {
			C = 0,
			N = 1 << 0,
			NE = 1 << 1,
			E = 1 << 2,
			SE = 1 << 3,
			S = 1 << 4,
			SW = 1 << 5,
			W = 1 << 6,
			NW = 1 << 7,
		};

		struct orientationPair {
			int x;
			int y;
			orientationSymbol symbol;

			bool main_orientation() const {
				return (abs(x) + abs(y)) == 1;
			}
			
		};
		inline bool operator<(const orientationPair& lhs, const orientationPair& rhs)
		{
			return lhs.symbol <rhs.symbol;
		};

		LX_GEO_FACTORY_SHARED_API const std::map<orientationSymbol, orientationPair> orientations_map = {
			{orientationSymbol::N, { 0,1,orientationSymbol::N }},
			{orientationSymbol::NE, { 1,1,orientationSymbol::NE }},
			{orientationSymbol::E, { 1,0,orientationSymbol::E }},
			{orientationSymbol::SE, { 1,-1,orientationSymbol::SE }},
			{orientationSymbol::S, { 0,-1,orientationSymbol::S }},
			{orientationSymbol::SW, { -1,-1,orientationSymbol::SW }},
			{orientationSymbol::W, { -1,0,orientationSymbol::W }},
			{orientationSymbol::NW, { -1,1,orientationSymbol::NW }},
			{orientationSymbol::C, { 0,0,orientationSymbol::C }}
		};

		
		LX_GEO_FACTORY_SHARED_API const std::map<orientationSymbol, std::set<orientationSymbol>> main_secondary_orientations_relation = {
			{orientationSymbol::N, {orientationSymbol::NE, orientationSymbol::NW}},
			{orientationSymbol::S, {orientationSymbol::SE, orientationSymbol::SW}},
			{orientationSymbol::E, {orientationSymbol::NE, orientationSymbol::SE}},
			{orientationSymbol::W, {orientationSymbol::NE, orientationSymbol::NW}}
		};
		LX_GEO_FACTORY_SHARED_API const std::map<orientationSymbol, std::set<orientationSymbol>> secondary_main_orientations_relation = {
			{orientationSymbol::NE, {orientationSymbol::N, orientationSymbol::E}},
			{orientationSymbol::NW, {orientationSymbol::N, orientationSymbol::W}},
			{orientationSymbol::SE, {orientationSymbol::S, orientationSymbol::E}},
			{orientationSymbol::SW, {orientationSymbol::S, orientationSymbol::W}}
		};

		LX_GEO_FACTORY_SHARED_API const std::map<std::pair<int, int>, orientationSymbol> dxy_map{
			{{0,1}, orientationSymbol::N},
			{{1,1}, orientationSymbol::NE},
			{{1,0}, orientationSymbol::E},
			{{1,-1}, orientationSymbol::SE},
			{{0,-1}, orientationSymbol::S},
			{{-1,-1}, orientationSymbol::SW},
			{{-1,0}, orientationSymbol::W},
			{{-1,1}, orientationSymbol::NW},
			{{0,0}, orientationSymbol::C},
		};


		LX_GEO_FACTORY_SHARED_API const orientationPair getOrientation(int& x1, int& y1, int& x2, int& y2) {
			int NS = sign(y2 - y1);
			int EW = sign(x2 - x1);
			auto symbol = dxy_map.at(std::make_pair(NS, EW));
			return orientations_map.at(symbol);
		};

	}
}