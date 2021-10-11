#ifndef GPWE_OCTREE_HPP
#define GPWE_OCTREE_HPP 1

#include <cassert>
#include <cstdint>
#include <variant>
#include <array>

#include "Vector.hpp"

namespace gpwe{
	template<typename LeafData>
	class OctreeNode{
		public:
			using ChildIndices = std::array<std::int32_t, 8>;
			struct LeafT{} Leaf;
			struct BranchT{} Branch;

			OctreeNode(): m_val(std::in_place_type<std::monostate>){}

			template<typename ... Args>
			OctreeNode(LeafT, Args &&... args)
				: m_val(
					std::in_place_type<LeafData>,
					std::forward<Args>(args)...
				){}

			OctreeNode(BranchT, const ChildIndices &indices)
				: m_val(std::in_place_type<ChildIndices>, indices){}

			OctreeNode(BranchT): OctreeNode(Branch, {-1, -1, -1, -1, -1, -1, -1, -1}){}

			OctreeNode(const OctreeNode&) = default;
			OctreeNode(OctreeNode&&) = default;

			~OctreeNode(){}

			OctreeNode &operator=(const OctreeNode&) = default;
			OctreeNode &operator=(OctreeNode&&) = default;

			bool isBranch() const{ return std::holds_alternative<ChildIndices>(m_val); }
			bool isLeaf() const{ return std::holds_alternative<LeafData>(m_val); }

			// max 8
			std::uint8_t numChildren(){
				auto children = std::get_if<ChildIndices>(&m_val);
				if(!children) return 0;

				std::uint8_t n = 0;
				for(auto child : *children){
					if(child >= 0) ++n;
				}

				return n;
			}

			bool setChildIndex(std::uint8_t child, std::int32_t idx){
				if(child >= 8) return false;

				auto children = std::get_if<ChildIndices>(&m_val);
				if(!children) return false;

				(*children)[child] = idx;
				return true;
			}

			std::int32_t childIndex(std::uint8_t child) const{
				if(child >= 8) return -1;

				auto children = std::get_if<ChildIndices>(&m_val);
				if(!children) return false;

				return (*children)[child];
			}

			LeafData *value() noexcept{ return std::get_if<LeafData>(&m_val); }
			const LeafData *value() const noexcept{ return std::get_if<LeafData>(&m_val); }

		private:
			std::variant<std::monostate, LeafData, ChildIndices> m_val;
	};

	template<typename LeafData, std::uint16_t NumLevels = 8>
	class Octree{
		public:
			static_assert(NumLevels > 1);

			using Node = OctreeNode<LeafData>;

			Octree(){}

			Node *emplaceNode(const Vector<std::uint8_t> &indices){
				if(indices.empty()) return nullptr;

				auto rootIdx = indices[0];
				if(rootIdx >= 8) return nullptr;

				auto &&rootNode = m_root[rootIdx];

				auto cur = &rootNode;

				for(std::size_t i = 1; i < indices.size(); i++){
					auto &&level = m_levels[i-1];

					auto idx = indices[i];
					auto childIdx = cur->childIndex(idx);
					if(childIdx == -1){
						// TODO: emplace node here
					}
				}

				return cur;
			}

		private:
			std::array<Node, NumLevels> m_root;
			std::array<Vector<Node>, NumLevels> m_levels;
	};
}

#endif // !GPWE_OCTREE_HPP
