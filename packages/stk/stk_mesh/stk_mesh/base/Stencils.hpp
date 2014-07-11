/*------------------------------------------------------------------------*/
/*                 Copyright 2010, 2011 Sandia Corporation.                     */
/*  Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive   */
/*  license for use of this work by or on behalf of the U.S. Government.  */
/*  Export of this program may require a license from the                 */
/*  United States Government.                                             */
/*------------------------------------------------------------------------*/

#ifndef stk_mesh_Stencils_hpp
#define stk_mesh_Stencils_hpp

#include <stddef.h>                     // for size_t
#include <stk_mesh/base/Types.hpp>      // for EntityRank, etc
#include "stk_topology/topology.hpp"    // for topology, etc



namespace stk {
namespace mesh {

relation_stencil_ptr get_element_node_stencil(size_t spatial_dimension);

template<class TopologyTraits, EntityRank element_rank >
int element_node_stencil( EntityRank , EntityRank , unsigned );


template<class TopologyTraits, EntityRank element_rank >
int element_node_stencil( EntityRank from_type , EntityRank to_type , unsigned identifier )
{
  enum { number_node = TopologyTraits::node_count };

  int ordinal = -1 ;

  if ( element_rank == from_type &&
       stk::topology::NODE_RANK == to_type &&
       identifier < number_node ) {
    ordinal = static_cast<int>(identifier);
  }

  return ordinal ;
}

} // namespace mesh
} // namespace stk

#endif //  stk_mesh_Stencils_hpp