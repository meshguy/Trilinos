// Copyright (c) 2013, Sandia Corporation.
// Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
// the U.S. Government retains certain rights in this software.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// 
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
// 
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
// 
//     * Neither the name of Sandia Corporation nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 

#include <stddef.h>                     // for size_t
#include <stk_mesh/base/BulkData.hpp>   // for BulkData
#include <stk_mesh/base/GetEntities.hpp>       // for comm_mesh_counts, count_entities
#include <stk_mesh/base/Comm.hpp>       // for comm_mesh_counts
#include <stk_mesh/base/MetaData.hpp>   // for MetaData
#include <gtest/gtest.h>
#include <vector>                       // for vector, vector<>::iterator
#include "stk_mesh/base/Bucket.hpp"     // for Bucket
#include "stk_mesh/base/Types.hpp"      // for BucketVector, EntityRank
#include "stk_topology/topology.hpp"    // for topology, etc
#include "unit_tests/Setup2Block2HexMesh.hpp"
#include "stk_mesh/baseImpl/MeshImplUtils.hpp"
#include "stk_io/StkMeshIoBroker.hpp"
#include "unit_tests/UnitTestMeshUtils.hpp"

using stk::mesh::MetaData;
using stk::mesh::BulkData;
using stk::mesh::Selector;
using stk::mesh::Entity;
using stk::mesh::EntityKey;
using stk::mesh::EntityVector;
using stk::mesh::EntityRank;
using stk::mesh::impl::VisitClosureGeneral;
using stk::mesh::impl::VisitClosure;
using stk::mesh::impl::VisitUpwardClosureGeneral;
using stk::mesh::impl::VisitUpwardClosure;
using stk::mesh::impl::VisitAuraClosureGeneral;
using stk::mesh::impl::VisitAuraClosure;
using stk::mesh::impl::StoreInVector;
using stk::mesh::impl::StoreInSet;
using stk::mesh::impl::AlwaysVisit;
using stk::mesh::impl::OnlyVisitOnce;
using stk::mesh::impl::OnlyVisitGhostsOnce;
using stk::mesh::impl::OnlyVisitLocallyOwnedOnce;
using stk::mesh::impl::OnlyVisitSharedOnce;

TEST ( MeshImplUtils, find_elements_these_nodes_have_in_common )
{
  stk::ParallelMachine communicator = MPI_COMM_WORLD;
  int numProcs = stk::parallel_machine_size(communicator);
  if (numProcs > 2) {
    return;
  }

  const unsigned spatialDim = 3;
  stk::mesh::MetaData meta(spatialDim);
  stk::mesh::BulkData bulk(meta, communicator);

  setup2Block2HexMesh(bulk);

  const unsigned numNodesPerEdge = 2;

  //edge 2-6 is connected to elements 1 and 2
  stk::mesh::EntityId edge_2_6_nodeIds[] = {2, 6};
  stk::mesh::Entity edge_2_6_nodes[numNodesPerEdge];
  edge_2_6_nodes[0] = bulk.get_entity(stk::topology::NODE_RANK, edge_2_6_nodeIds[0]);
  edge_2_6_nodes[1] = bulk.get_entity(stk::topology::NODE_RANK, edge_2_6_nodeIds[1]);

  //edge 5-6 is only connected to element 1
  stk::mesh::EntityId edge_5_6_nodeIds[] = {5, 6};
  stk::mesh::Entity edge_5_6_nodes[numNodesPerEdge];
  edge_5_6_nodes[0] = bulk.get_entity(stk::topology::NODE_RANK, edge_5_6_nodeIds[0]);
  edge_5_6_nodes[1] = bulk.get_entity(stk::topology::NODE_RANK, edge_5_6_nodeIds[1]);

  std::vector<stk::mesh::Entity> elements;

  stk::mesh::impl::find_elements_these_nodes_have_in_common(bulk, numNodesPerEdge, edge_2_6_nodes, elements);

  size_t expected_num_elements = 2;
  EXPECT_EQ(expected_num_elements, elements.size());

  stk::mesh::impl::find_elements_these_nodes_have_in_common(bulk, numNodesPerEdge, edge_5_6_nodes, elements);

  expected_num_elements = 1;
  EXPECT_EQ(expected_num_elements, elements.size());
}

TEST ( MeshImplUtils, find_locally_owned_elements_these_nodes_have_in_common )
{
  stk::ParallelMachine communicator = MPI_COMM_WORLD;
  int numProcs = stk::parallel_machine_size(communicator);
  if (numProcs > 2) {
    return;
  }

  const unsigned spatialDim = 3;
  stk::mesh::MetaData meta(spatialDim);
  stk::mesh::BulkData bulk(meta, communicator);

  setup2Block2HexMesh(bulk);

  const unsigned numNodesPerEdge = 2;

  //edge 2-6 is connected to elements 1 and 2
  stk::mesh::EntityId edge_2_6_nodeIds[] = {2, 6};
  stk::mesh::Entity edge_2_6_nodes[numNodesPerEdge];
  edge_2_6_nodes[0] = bulk.get_entity(stk::topology::NODE_RANK, edge_2_6_nodeIds[0]);
  edge_2_6_nodes[1] = bulk.get_entity(stk::topology::NODE_RANK, edge_2_6_nodeIds[1]);

  //edge 5-6 is only connected to element 1
  stk::mesh::EntityId edge_5_6_nodeIds[] = {5, 6};
  stk::mesh::Entity edge_5_6_nodes[numNodesPerEdge];
  edge_5_6_nodes[0] = bulk.get_entity(stk::topology::NODE_RANK, edge_5_6_nodeIds[0]);
  edge_5_6_nodes[1] = bulk.get_entity(stk::topology::NODE_RANK, edge_5_6_nodeIds[1]);

  std::vector<stk::mesh::Entity> elements;

  stk::mesh::impl::find_locally_owned_elements_these_nodes_have_in_common(bulk, numNodesPerEdge, edge_2_6_nodes, elements);

  size_t expected_num_elements = 1;
  if (numProcs == 1) {
    expected_num_elements = 2;
  }
  EXPECT_EQ(expected_num_elements, elements.size());

  stk::mesh::impl::find_locally_owned_elements_these_nodes_have_in_common(bulk, numNodesPerEdge, edge_5_6_nodes, elements);

  expected_num_elements = 0;
  if (bulk.parallel_rank() == 0) {
    //edge_5_6 is connected to element 1, which is locally-owned on proc 0
    expected_num_elements = 1;
  }
  EXPECT_EQ(expected_num_elements, elements.size());
}

TEST ( MeshImplUtils, find_element_edge_ordinal_and_equivalent_nodes )
{
  stk::ParallelMachine communicator = MPI_COMM_WORLD;
  int numProcs = stk::parallel_machine_size(communicator);
  if (numProcs > 2) {
    return;
  }

  const unsigned spatialDim = 3;
  stk::mesh::MetaData meta(spatialDim);
  stk::mesh::BulkData bulk(meta, communicator);

  setup2Block2HexMesh(bulk);

  const unsigned numNodesPerEdge = 2;

  //we know that edge 2-6 is edge-ordinal 9 on element 1 and
  //edge-ordinal 8 on element 2
  stk::mesh::EntityId edge_2_6_nodeIds[] = {2, 6};
  stk::mesh::Entity edge_2_6_nodes[numNodesPerEdge];
  edge_2_6_nodes[0] = bulk.get_entity(stk::topology::NODE_RANK, edge_2_6_nodeIds[0]);
  edge_2_6_nodes[1] = bulk.get_entity(stk::topology::NODE_RANK, edge_2_6_nodeIds[1]);

  stk::mesh::EntityId elem1Id = 1;
  stk::mesh::EntityId elem2Id = 2;
  stk::mesh::Entity elem1 = bulk.get_entity(stk::topology::ELEM_RANK, elem1Id);
  stk::mesh::Entity elem2 = bulk.get_entity(stk::topology::ELEM_RANK, elem2Id);

  stk::mesh::Entity elemEdgeNodes[numNodesPerEdge];
  unsigned elemEdgeOrdinal = 999;

  bool found_it = stk::mesh::impl::find_element_edge_ordinal_and_equivalent_nodes(bulk, elem1, numNodesPerEdge, edge_2_6_nodes,
                      elemEdgeOrdinal, elemEdgeNodes);

  EXPECT_EQ(true, found_it);
  unsigned expectedElemEdgeOrdinal = 9;
  EXPECT_EQ(expectedElemEdgeOrdinal, elemEdgeOrdinal);
  EXPECT_EQ(edge_2_6_nodes[0], elemEdgeNodes[0]);
  EXPECT_EQ(edge_2_6_nodes[1], elemEdgeNodes[1]);

  found_it = stk::mesh::impl::find_element_edge_ordinal_and_equivalent_nodes(bulk, elem2, numNodesPerEdge, edge_2_6_nodes,
                      elemEdgeOrdinal, elemEdgeNodes);

  EXPECT_EQ(true, found_it);
  expectedElemEdgeOrdinal = 8;
  EXPECT_EQ(expectedElemEdgeOrdinal, elemEdgeOrdinal);
  EXPECT_EQ(edge_2_6_nodes[0], elemEdgeNodes[0]);
  EXPECT_EQ(edge_2_6_nodes[1], elemEdgeNodes[1]);
}

class ClosureFixture
{
public:
    ClosureFixture(MPI_Comm communicator, int num_x, int num_y=1)
    {
        const int spatialDim = 3;
        m_meta = new stk::mesh::MetaData(spatialDim);
        m_mesh = new stk::mesh::BulkData(*m_meta,communicator);
        std::ostringstream oss;
        oss << "generated:" << num_x << "x" << num_y << "x" << m_mesh->parallel_size() << "|sideset:xXyYzZ";
        std::string exodusFileName = getOption("-i", oss.str());
        stk::io::StkMeshIoBroker exodus_file_reader(communicator);
        exodus_file_reader.set_bulk_data(*m_mesh);
        exodus_file_reader.add_mesh_database(exodusFileName, stk::io::READ_MESH);
        exodus_file_reader.create_input_mesh();
        exodus_file_reader.populate_bulk_data();
    }
    ~ClosureFixture()
    {
        delete(m_mesh);
        delete(m_meta);
    }
    void WriteToExodusFile(std::string filename)
    {
        stk::io::StkMeshIoBroker exodus_file_reader(m_mesh->parallel());
        exodus_file_reader.set_bulk_data(*m_mesh);
        int index = exodus_file_reader.create_output_mesh(filename, stk::io::WRITE_RESULTS);
        exodus_file_reader.write_output_mesh(index);
    }

    BulkData & mesh() { return *m_mesh; }
    int prank() { return m_mesh->parallel_rank(); }
    int psize() { return m_mesh->parallel_size(); }

private:
    MetaData * m_meta;
    BulkData * m_mesh;
};


std::string PrintEntityVector(EntityVector ev, const BulkData & mesh)
{
  std::ostringstream oss;
  int myRank = mesh.parallel_rank();
  oss << "P" << myRank << " { ";
  for (size_t i=0 ; i<ev.size() ; ++i) {
      EntityKey key = mesh.entity_key(ev[i]);
      oss << key << " ";
  }
  oss << "}\n";
  return oss.str();
}

TEST(MeshImplUtils, visit_closure_trivial)
{
    MPI_Comm communicator = MPI_COMM_WORLD;
    const int spatialDim = 3;
    MetaData meta(spatialDim);
    BulkData mesh(meta, communicator);

    Entity entity = Entity();
    EntityVector ev;
    StoreInVector<EntityVector> siv(ev);
    VisitClosure(mesh, entity, siv);
    EXPECT_EQ( 1u, ev.size() );
}

TEST(MeshImplUtils, visit_closure_nominal)

{
    MPI_Comm communicator = MPI_COMM_WORLD;
    ClosureFixture fix(communicator,2);
    int numProcs = fix.psize();
    const int myRank = fix.prank();
    BulkData & mesh = fix.mesh();

    Selector locally_owned_selector = mesh.mesh_meta_data().locally_owned_part();
    EntityVector element_vector;
    get_selected_entities(locally_owned_selector, mesh.buckets(stk::topology::ELEMENT_RANK),element_vector);
    ASSERT_TRUE( !element_vector.empty() );
    Entity element = element_vector[0];
    EntityVector ev;
    StoreInVector<EntityVector> siv(ev);
    VisitClosure(mesh, element, siv);
    if (numProcs == 1)
    {
        EXPECT_EQ( 14u, ev.size() );
    }
    else if (myRank == 0 || myRank == numProcs -1)
    {
        EXPECT_EQ( 13u, ev.size() );
    }
    else
    {
        EXPECT_EQ( 12u, ev.size() );
    }
}

TEST(MeshImplUtils, visit_closure_face)
{
    MPI_Comm communicator = MPI_COMM_WORLD;
    ClosureFixture fix(communicator,2);
    BulkData & mesh = fix.mesh();

    EntityVector ev;
    StoreInVector<EntityVector> siv(ev);
    Selector locally_owned_selector = mesh.mesh_meta_data().locally_owned_part();
    EntityVector face_vector;
    get_selected_entities(locally_owned_selector, mesh.buckets(stk::topology::FACE_RANK),face_vector);
    ASSERT_TRUE( !face_vector.empty() );
    Entity face = face_vector[0];
    VisitClosure(mesh, face, siv);
    EXPECT_EQ( 5u, ev.size() );
}

TEST(MeshImplUtils, visit_closure_of_vector)
{
    MPI_Comm communicator = MPI_COMM_WORLD;
    ClosureFixture fix(communicator,2);
    int numProcs = fix.psize();
    const int myRank = fix.prank();
    BulkData & mesh = fix.mesh();

    EntityVector ev;
    StoreInVector<EntityVector> siv(ev);
    Selector locally_owned_selector = mesh.mesh_meta_data().locally_owned_part();
    EntityVector element_vector;
    get_selected_entities(locally_owned_selector, mesh.buckets(stk::topology::ELEMENT_RANK),element_vector);
    VisitClosure(mesh,element_vector.begin(),element_vector.end(),siv);
    if (numProcs == 1)
    {
        EXPECT_EQ( 24u, ev.size() );
    }
    else if (myRank == 0 || myRank == numProcs-1)
    {
        EXPECT_EQ( 22u, ev.size() );
    }
    else
    {
        EXPECT_EQ( 20u, ev.size() );
    }
}

TEST(MeshImplUtils, visit_closure_of_vector_locally_owned)
{
    MPI_Comm communicator = MPI_COMM_WORLD;
    ClosureFixture fix(communicator,2);
    int numProcs = fix.psize();
    const int myRank = fix.prank();
    BulkData & mesh = fix.mesh();

    EntityVector ev;
    StoreInVector<EntityVector> siv(ev);
    Selector locally_owned_selector = mesh.mesh_meta_data().locally_owned_part();
    EntityVector element_vector;
    get_selected_entities(locally_owned_selector, mesh.buckets(stk::topology::ELEMENT_RANK),element_vector);
    OnlyVisitLocallyOwnedOnce ovloeo(mesh);
    VisitClosureGeneral(mesh,element_vector.begin(),element_vector.end(),siv,ovloeo);
    if (numProcs == 1)
    {
        EXPECT_EQ( 24u, ev.size() );
    }
    else if (myRank == 0)
    {
        EXPECT_EQ( 22u, ev.size() );
    }
    else if (myRank == numProcs-1)
    {
        EXPECT_EQ( 16u, ev.size() );
    }
    else
    {
        EXPECT_EQ( 14u, ev.size() );
    }
}

TEST(MeshImplUtils, visit_upward_closure_trivial)
{
    MPI_Comm communicator = MPI_COMM_WORLD;
    const int spatialDim = 3;
    MetaData meta(spatialDim);
    BulkData mesh(meta, communicator);

    Entity entity = Entity();
    EntityVector ev;
    StoreInVector<EntityVector> siv(ev);
    VisitUpwardClosure(mesh, entity, siv);
    EXPECT_EQ( 1u, ev.size() );
}

TEST(MeshImplUtils, visit_upward_closure_nominal)
{
    MPI_Comm communicator = MPI_COMM_WORLD;
    ClosureFixture fix(communicator,2);
    int numProcs = fix.psize();
    const int myRank = fix.prank();
    BulkData & mesh = fix.mesh();
    if (numProcs != 4) { return; }
    fix.WriteToExodusFile("2x1x4.exo");

    Entity node;
    if (myRank == 0) {
        node = mesh.get_entity(stk::topology::NODE_RANK,1);
    }
    else if (myRank == 1) {
        node = mesh.get_entity(stk::topology::NODE_RANK,7);
    }
    else if (myRank == 2) {
        node = mesh.get_entity(stk::topology::NODE_RANK,17);
    }
    else if (myRank == 3) {
        node = mesh.get_entity(stk::topology::NODE_RANK,29);
    }
    EntityVector ev;
    StoreInVector<EntityVector> siv(ev);
    VisitUpwardClosure(mesh, node, siv);
    if (myRank == 0) {
        EXPECT_EQ( 5u, ev.size() );
    }
    else if (myRank == 1) {
        EXPECT_EQ( 7u, ev.size() );
    }
    else if (myRank == 2) {
        EXPECT_EQ( 9u, ev.size() );
    }
    else { // myRank == 3
        EXPECT_EQ( 7u, ev.size() );
    }
}

TEST(MeshImplUtils, visit_upward_closure_face)
{
    MPI_Comm communicator = MPI_COMM_WORLD;
    ClosureFixture fix(communicator,2);
    BulkData & mesh = fix.mesh();

    EntityVector ev;
    StoreInVector<EntityVector> siv(ev);
    Selector locally_owned_selector = mesh.mesh_meta_data().locally_owned_part();
    EntityVector face_vector;
    get_selected_entities(locally_owned_selector, mesh.buckets(stk::topology::FACE_RANK),face_vector);
    ASSERT_TRUE( !face_vector.empty() );
    Entity face = face_vector[0];
    VisitUpwardClosure(mesh, face, siv);
    EXPECT_EQ( 2u, ev.size() );
}

TEST(MeshImplUtils, visit_upward_closure_of_vector)
{
    MPI_Comm communicator = MPI_COMM_WORLD;
    ClosureFixture fix(communicator,2);
    int numProcs = fix.psize();
    const int myRank = fix.prank();
    BulkData & mesh = fix.mesh();
    if (numProcs != 4) { return; }

    EntityVector entity_vector;
    if (myRank == 0) {
      entity_vector.push_back(mesh.get_entity(stk::topology::NODE_RANK,1));
      entity_vector.push_back(mesh.get_entity(stk::topology::NODE_RANK,6));
    }
    else if (myRank == 1) {
      entity_vector.push_back(mesh.get_entity(stk::topology::ELEMENT_RANK,3));
    }
    else if (myRank == 2) {
      entity_vector.push_back(mesh.get_entity(stk::topology::ELEMENT_RANK,5));
      entity_vector.push_back(mesh.get_entity(stk::topology::NODE_RANK,15));
    }
    else { // (myRank == 3)
      entity_vector.push_back(mesh.get_entity(stk::topology::NODE_RANK,20));
      entity_vector.push_back(mesh.get_entity(stk::topology::NODE_RANK,23));
    }

    EntityVector ev;
    StoreInVector<EntityVector> siv(ev);
    VisitUpwardClosure(mesh,entity_vector.begin(),entity_vector.end(),siv);
    if (myRank == 0) {
        EXPECT_EQ( 10u, ev.size() );
    }
    else if (myRank == 1) {
        EXPECT_EQ( 1u, ev.size() );
    }
    else if (myRank == 2) {
        EXPECT_EQ( 8u, ev.size() );
    }
    else { // (myRank == 3)
        EXPECT_EQ( 14u, ev.size() );
    }
}

TEST(MeshImplUtils, visit_aura_closure_trivial)
{
    MPI_Comm communicator = MPI_COMM_WORLD;
    ClosureFixture fix(communicator,2);
    BulkData & mesh = fix.mesh();

    EntityVector ev;
    StoreInVector<EntityVector> siv(ev);
    Entity entity = Entity();
    VisitAuraClosure(mesh,entity,siv);
    EXPECT_EQ( 1u, ev.size() );
}

TEST(MeshImplUtils, visit_aura_closure_of_element)
{
    MPI_Comm communicator = MPI_COMM_WORLD;
    ClosureFixture fix(communicator,2);
    int numProcs = fix.psize();
    const int myRank = fix.prank();
    BulkData & mesh = fix.mesh();
    if (numProcs > 4) { return; }

    EntityVector ev;
    StoreInVector<EntityVector> siv(ev);
    Selector locally_owned_selector = mesh.mesh_meta_data().locally_owned_part();
    EntityVector element_vector;
    get_selected_entities(locally_owned_selector, mesh.buckets(stk::topology::ELEMENT_RANK),element_vector);
    ASSERT_TRUE( !element_vector.empty() );
    Entity element = element_vector[0];
    VisitAuraClosure(mesh,element,siv);
    if (numProcs == 1)
    {
        EXPECT_EQ( 24u, ev.size() );
    }
    else if (numProcs == 2)
    {
        EXPECT_EQ( 38u, ev.size() );
    }
    else if (myRank == 0 || myRank == numProcs-1)
    {
        EXPECT_EQ( 36u, ev.size() );
    }
    else
    {
        if (numProcs==3) {
            EXPECT_EQ( 52u, ev.size() );
        }
        else {
            EXPECT_EQ( 50u, ev.size() );
        }
    }

}

TEST(MeshImplUtils, visit_aura_closure_of_corner_node)
{
    MPI_Comm communicator = MPI_COMM_WORLD;
    ClosureFixture fix(communicator,2);
    int numProcs = fix.psize();
    BulkData & mesh = fix.mesh();
    if (numProcs > 1) { return; }

    Entity node;
    Selector locally_owned_selector = mesh.mesh_meta_data().locally_owned_part();
    EntityVector node_vector;
    get_selected_entities(locally_owned_selector, mesh.buckets(stk::topology::NODE_RANK),node_vector);
    ASSERT_TRUE( !node_vector.empty() );
    node = node_vector[0];
    EntityVector ev;
    StoreInVector<EntityVector> siv(ev);
    VisitAuraClosure(mesh,node,siv);
    EXPECT_EQ( 14u, ev.size() );
}

TEST(MeshImplUtils, visit_aura_closure_of_center_node)
{
    MPI_Comm communicator = MPI_COMM_WORLD;
    ClosureFixture fix(communicator,2);
    int numProcs = fix.psize();
    BulkData & mesh = fix.mesh();
    if (numProcs > 1) { return; }

    Entity node;
    Selector locally_owned_selector = mesh.mesh_meta_data().locally_owned_part();
    EntityVector node_vector;
    get_selected_entities(locally_owned_selector, mesh.buckets(stk::topology::NODE_RANK),node_vector);
    ASSERT_TRUE( node_vector.size() > 1 );
    node = node_vector[1];
    EntityVector ev;
    StoreInVector<EntityVector> siv(ev);
    VisitAuraClosure(mesh,node,siv);
    EXPECT_EQ( 24u, ev.size() );
}

TEST(MeshImplUtils, visit_aura_closure_of_corner_node_in_2procs)
{
    MPI_Comm communicator = MPI_COMM_WORLD;
    ClosureFixture fix(communicator,2);
    int numProcs = fix.psize();
    BulkData & mesh = fix.mesh();
    if (numProcs != 2 ) { return; }

    Selector shared_selector = mesh.mesh_meta_data().globally_shared_part();
    EntityVector node_vector;
    get_selected_entities(shared_selector, mesh.buckets(stk::topology::NODE_RANK),node_vector);
    ASSERT_TRUE( !node_vector.empty() );
    Entity node = node_vector[0];
    EntityVector ev;
    StoreInVector<EntityVector> siv(ev);
    VisitAuraClosure(mesh,node,siv);
    EXPECT_EQ( 22u, ev.size() );
}

TEST(MeshImplUtils, visit_aura_closure_of_center_node_in_2procs)
{
    MPI_Comm communicator = MPI_COMM_WORLD;
    ClosureFixture fix(communicator,2);
    int numProcs = fix.psize();
    BulkData & mesh = fix.mesh();
    if (numProcs != 2 ) { return; }

    Selector shared_selector = mesh.mesh_meta_data().globally_shared_part();
    EntityVector node_vector;
    get_selected_entities(shared_selector, mesh.buckets(stk::topology::NODE_RANK),node_vector);
    ASSERT_TRUE( node_vector.size() > 1 );
    Entity node = node_vector[1];
    EntityVector ev;
    StoreInVector<EntityVector> siv(ev);
    VisitAuraClosure(mesh,node,siv);
    EXPECT_EQ( 38u, ev.size() );
}

TEST(MeshImplUtils, visit_aura_closure_of_side_node_in_3procs)
{
    MPI_Comm communicator = MPI_COMM_WORLD;
    ClosureFixture fix(communicator,4,4); // 4 x 4 x 3
    int numProcs = fix.psize();
    const int myRank = fix.prank();
    BulkData & mesh = fix.mesh();
    if (numProcs != 3 ) { return; }

    Entity node;
    if (myRank == 0 || myRank == 1) {
        node = mesh.get_entity(stk::topology::NODE_RANK,31);
    }
    else  {
        node = mesh.get_entity(stk::topology::NODE_RANK,56);
    }
    EntityVector ev;
    StoreInVector<EntityVector> siv(ev);
    VisitAuraClosure(mesh,node,siv);
    EXPECT_EQ( 30u, ev.size() );
}

TEST(MeshImplUtils, visit_aura_closure_of_center_node_in_3procs)
{
    MPI_Comm communicator = MPI_COMM_WORLD;
    ClosureFixture fix(communicator,4,4); // 4 x 4 x 3
    int numProcs = fix.psize();
    const int myRank = fix.prank();
    BulkData & mesh = fix.mesh();
    if (numProcs != 3 ) { return; }

    Entity node;
    if (myRank == 0 || myRank == 1) {
        node = mesh.get_entity(stk::topology::NODE_RANK,32);
    }
    else  {
        node = mesh.get_entity(stk::topology::NODE_RANK,57);
    }
    EntityVector ev;
    StoreInVector<EntityVector> siv(ev);
    VisitAuraClosure(mesh,node,siv);
    EXPECT_EQ( 47u, ev.size() );
}

TEST(MeshImplUtils, visit_aura_closure_of_side_element_in_3procs)
{
    MPI_Comm communicator = MPI_COMM_WORLD;
    ClosureFixture fix(communicator,4,4); // 4 x 4 x 3
    int numProcs = fix.psize();
    const int myRank = fix.prank();
    BulkData & mesh = fix.mesh();
    if (numProcs != 3 ) { return; }

    Entity element = mesh.get_entity(stk::topology::ELEMENT_RANK,21);
    EntityVector ev;
    StoreInVector<EntityVector> siv(ev);
    VisitAuraClosure(mesh,element,siv);
    if (myRank == 0 || myRank == 2) {
        EXPECT_EQ( 64u, ev.size() );
    }
    else {
        EXPECT_EQ( 93u, ev.size() );
    }
}

TEST(MeshImplUtils, visit_aura_closure_of_center_element_in_3procs)
{
    MPI_Comm communicator = MPI_COMM_WORLD;
    ClosureFixture fix(communicator,4,4); // 4 x 4 x 3
    int numProcs = fix.psize();
    const int myRank = fix.prank();
    BulkData & mesh = fix.mesh();
    if (numProcs != 3 ) { return; }

    Entity element = mesh.get_entity(stk::topology::ELEMENT_RANK,22);
    EntityVector ev;
    StoreInVector<EntityVector> siv(ev);
    VisitAuraClosure(mesh,element,siv);
    if (myRank == 0 || myRank == 2) {
        EXPECT_EQ( 87u, ev.size() );
    }
    else  {
        EXPECT_EQ( 127u, ev.size() );
    }
}

TEST(MeshImplUtils, visit_aura_closure_vector)
{
    MPI_Comm communicator = MPI_COMM_WORLD;
    ClosureFixture fix(communicator,4,4); // 4x4x1
    int numProcs = fix.psize();
    BulkData & mesh = fix.mesh();
    if (numProcs != 1 ) { return; }

    std::vector<Entity> node_vec;
    node_vec.push_back(mesh.get_entity(stk::topology::NODE_RANK,25));
    node_vec.push_back(mesh.get_entity(stk::topology::NODE_RANK,26));
    EntityVector ev;
    StoreInVector<EntityVector> siv(ev);
    VisitAuraClosure(mesh,node_vec.begin(),node_vec.end(),siv);
    EXPECT_EQ( 26u, ev.size() );
}


TEST(MeshImplUtils, visit_aura_closure_vector_ghost)
{
    MPI_Comm communicator = MPI_COMM_WORLD;
    ClosureFixture fix(communicator,1,1); // 1x1x4
    const int myRank = fix.prank();
    int numProcs = fix.psize();
    BulkData & mesh = fix.mesh();
    if (numProcs != 4 ) { return; }

    EntityVector ev;
    if (myRank > 0) {
        ev.push_back(mesh.get_entity(stk::topology::NODE_RANK,13));
        ev.push_back(mesh.get_entity(stk::topology::NODE_RANK,14));
        ev.push_back(mesh.get_entity(stk::topology::NODE_RANK,15));
        ev.push_back(mesh.get_entity(stk::topology::NODE_RANK,16));
        ev.push_back(mesh.get_entity(stk::topology::ELEMENT_RANK,3));
        std::set<Entity> es;
        StoreInSet<std::set<Entity> > sis(es);
        OnlyVisitGhostsOnce ovgo(mesh);
        VisitAuraClosureGeneral(mesh,ev.begin(),ev.end(),sis,ovgo);
        if (myRank == 1) {
            EXPECT_EQ( 9u, es.size() );
        }
        else if (myRank == 2) {
            EXPECT_EQ( 19u, es.size() );
        }
        else if (myRank == 3) {
            EXPECT_EQ( 9u, es.size() );
        }
    }
}

//TEST(MeshImplUtils, internal_insert_transitive_ghost_closure)
//{
//    MPI_Comm communicator = MPI_COMM_WORLD;
//    ClosureFixture fix(communicator,1,1); // 1x1x4
//    const int myRank = fix.prank();
//    int numProcs = fix.psize();
//    BulkData & mesh = fix.mesh();
//    if (numProcs != 4 ) { return; }
//
//    EntityVector ev;
//    if (myRank > 0) {
//        ev.push_back(mesh.get_entity(stk::topology::NODE_RANK,13));
//        ev.push_back(mesh.get_entity(stk::topology::NODE_RANK,14));
//        ev.push_back(mesh.get_entity(stk::topology::NODE_RANK,15));
//        ev.push_back(mesh.get_entity(stk::topology::NODE_RANK,16));
//        ev.push_back(mesh.get_entity(stk::topology::ELEMENT_RANK,3));
//
//        std::set<EntityKey> work_list;
//        for (size_t i=0 ; i<ev.size() ; ++i) {
//            internal_insert_transitive_ghost(mesh,ev[i],mesh.parallel_rank(),work_list);
//        }
//
//        if (myRank == 1) {
//            EXPECT_EQ( 9u, work_list.size() );
//        }
//        else if (myRank == 2) {
//            EXPECT_EQ( 19u, work_list.size() );
//        }
//        else if (myRank == 3) {
//            EXPECT_EQ( 9u, work_list.size() );
//        }
//    }
//}
