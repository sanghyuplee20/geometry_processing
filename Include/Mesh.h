/*
Copyright (c) 2025, Michael Kazhdan
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of
conditions and the following disclaimer. Redistributions in binary form must reproduce
the above copyright notice, this list of conditions and the following disclaimer
in the documentation and/or other materials provided with the distribution. 

Neither the name of the Johns Hopkins University nor the names of its contributors
may be used to endorse or promote products derived from this software without specific
prior written permission. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO THE IMPLIED WARRANTIES 
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
TO, PROCUREMENT OF SUBSTITUTE  GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
DAMAGE.
*/

#pragma once

#include <optional>
#include <string>
#include <vector>
#include <unordered_map>
#include <Eigen/Sparse>
#include <Misha/MultiThreading.h>
#include <Misha/Ply.h>
#include <Misha/PlyVertexData.h>
#include <Misha/Geometry.h>
#include <Misha/Exceptions.h>

namespace MishaK
{
	namespace AdvancedGraphics
	{
		struct Mesh
		{
			// The scalar type
			using Real = double;

			// The dimension of the manifold
			static const unsigned int K = 2;

			// The embedding dimension of the manifold
			static const unsigned int Dim = 3;

			// The vertices of the mesh
			std::vector< Point< Real , Dim > > vertices;

			// The per-vertex normals of the mesh (may be zero-sized)
			std::vector< Point< Real , Dim > > normals;

			// The per-vertex colors of the mesh (may be zero-sized)
			std::vector< Point< Real , Dim > > colors;

			// The per-vertex values of the mesh (may be zero-sized)
			std::vector< Real > values;

			// The triangles the mesh
			std::vector< SimplexIndex< K > > triangles;

			// Default constructor
			Mesh( void );

			// Constructor from file
			Mesh( std::string fileName );

			// Reads from file (either as .ply or .obj)
			void read( std::string fileName );

			// Writes to file (either as .ply or .obj)
			void write( std::string fileName ) const;

			// Normalizes the mesh to have unit area
			void normalize( void );

			// Checks that the mesh is in a reasonable state
			// (Throws an exception if it's not)
			void validate( void ) const;

			// Returns the simplex defined by the t-th triangle
			Simplex< double , Dim , K > simplex( unsigned int t ) const;

			// Initialize the edge information
			void setEdges( void );

			// Returns the number of edges
			// [NOTE] Mesh::setEdges needs to have been called before invoking
			//        Otherwise an exception is thrown
			size_t numEdges( void ) const;

			// Returns the edge associated with the index
			// [NOTE] Mesh::setEdges needs to have been called before invoking
			//        Otherwise an exception is thrown
			std::pair< unsigned int , unsigned int > edge( unsigned int e ) const;

			// Returns the index associated with the directed edge (if it exists)
			// [NOTE] Mesh::setEdges needs to have been called before invoking
			//        Otherwise an exception is thrown
			std::optional< unsigned int > edgeIndex( std::pair< unsigned int , unsigned int > endPoints ) const;

			// Returns the index associated with the undirected edge (if it exists)
			// [NOTE] Mesh::setEdges needs to have been called before invoking
			//        Otherwise an exception is thrown
			std::optional< unsigned int > edgeIndex( std::pair< unsigned int , unsigned int > endPoints , bool &flip ) const;

		protected:
			template< bool HasNormals , bool HasValues >
			struct _PlyData;

			struct _EdgeInfo
			{
				struct Hasher{ size_t operator()( const std::pair< unsigned int , unsigned int > & ) const; };
				std::unordered_map< std::pair< unsigned int , unsigned int > , unsigned int , Hasher > edgeToIndex;
				std::vector< std::pair< unsigned int , unsigned int > > indexToEdge;

				_EdgeInfo( void );
				void set( const std::vector< SimplexIndex< K > > & triangles );
			};

			_EdgeInfo _edgeInfo;
			bool _edgesSet;

			template< bool HasNormals , bool HasValues >
			void _write( std::string fileName ) const;
		};
#include "Mesh.inl"
	}
}