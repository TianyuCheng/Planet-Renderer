#ifndef TILE_H
#define TILE_H

#include <memory>
#include <vector>
#include <functional>

#define TILE_DEBUG 0

#include "Vec2D.h"
#include "TileIO.h"
#include "TileInfo.h"

using std::unique_ptr;
using std::vector;

template<typename TileInfo>
class Tile {
public:
	typedef typename TileInfo::Coordinate Coord;
	typedef typename TileInfo::TileSeed Seed;
	typedef typename TileInfo::TileElement Element;

	Tile() = delete;
	Tile(const TileInfo& shape, const Seed& seed)
		:shape_(shape), seed_(seed)
	{
	}
	virtual ~Tile() {}

	bool write(const Coord& coord, const Element& elem)
	{
		ssize_t loc = shape_.get_linear(coord);
		if (loc < 0)
			return false;
		alloc_memory();
		elems_[loc] = elem;
		return true;
	}

	bool read(const Coord& coord, const Element& elem) const
	{
		ssize_t loc = shape_.get_linear(coord);
		if (loc < 0) {
			return false;
		}
		gen();
		elem = elems_[loc];
	}

	TileIO<Element> get_ioblock(const Coord& axesmins,
			const Coord& axesmaxs,
			int LODLevel)
	{
		gen();

		int istart, iend, jstart, jend;
		ssize_t start = shape_.get_linear(axesmins);
		auto imins = shape_.get_block(axesmins);
		auto imaxs = shape_.get_block(axesmaxs);
		if (imins.x < 0 || imaxs.x < 0)
			return TileIO<Element>(nullptr, 0, 0, 0);
		ssize_t lineelem = imaxs.y - imins.y;
		intptr_t stride = sizeof(Element) * lineelem;
		Element* buf = &elems_[start];
		return TileIO<Element>(buf, lineelem, stride, imaxs.x - imins.x);
	}

	const TileInfo& get_shape_info() const { return shape_; }
	Coord init_pos() const { return shape_.init_pos(); }
	Coord tail_pos() const { return shape_.tail_pos(); }
	double get_resolution(int LODLevel) const { return shape_.get_resolution(LODLevel); }

	void adjust_resolution(double newres)
	{
		elems_.clear();
		shape_.res = newres;
		alloc_memory();
	}
protected:
	TileInfo shape_;
	Seed seed_;
	mutable std::vector<Element> elems_;
	mutable std::vector<std::vector<Element>> lods_;

	void alloc_memory()
	{
		if (elems_.empty()) {
			size_t nelem = shape_.nelement();
			elems_.resize(nelem);
		}
	}

	void gen()
	{
		if (elems_.empty()) {
			alloc_memory();
			typename TileInfo::Generator generator(seed_);
			generator.gen(elems_);
		}
	}
};

/*
 * Application of class template Tile
 */
typedef Tile<TerrainTileInfo> TerrainTile;

#endif
