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

//////////
// Mesh //
//////////

inline Mesh::Mesh( void ) : _edgesSet(false) {}

inline Mesh::Mesh( std::string fileName ) : _edgesSet(false) { read(fileName); }

inline Simplex< double , Mesh::Dim , Mesh::K > Mesh::simplex( unsigned int t ) const
{
	Simplex< double , Dim , K > s;
	for( unsigned int k=0 ; k<=K ; k++ ) s[k] = vertices[ triangles[t][k] ];
	return s;
}

inline void Mesh::setEdges( void )
{
	_edgeInfo.set( triangles );
	_edgesSet = true;
}

inline size_t Mesh::numEdges( void ) const
{
	if( !_edgesSet ) MK_THROW( "Edges not set" );
	return _edgeInfo.indexToEdge.size();
}

inline std::pair< unsigned int , unsigned int > Mesh::edge( unsigned int e ) const
{
	if( !_edgesSet ) MK_THROW( "Edges not set" );
	return _edgeInfo.indexToEdge[e];
}

inline std::optional< unsigned int > Mesh::edgeIndex( std::pair< unsigned int , unsigned int > endPoints ) const
{
	if( !_edgesSet ) MK_THROW( "Edges not set" );
	auto iter = _edgeInfo.edgeToIndex.find( endPoints );
	if( iter!=_edgeInfo.edgeToIndex.end() ) return iter->second;
	return{};
}

inline std::optional< unsigned int > Mesh::edgeIndex( std::pair< unsigned int , unsigned int > endPoints , bool &flip ) const
{
	if( !_edgesSet ) MK_THROW( "Edges not set" );
	auto iter = _edgeInfo.edgeToIndex.find( endPoints );
	if( iter!=_edgeInfo.edgeToIndex.end() )
	{
		flip = false;
		return iter->second;
	}
	else
	{
		auto iter = _edgeInfo.edgeToIndex.find( std::make_pair( endPoints.second , endPoints.first ) );
		if( iter!=_edgeInfo.edgeToIndex.end() )
		{
			flip = true;
			return iter->second;
		}
	}
	return {};
}

inline void Mesh::read( std::string fileName )
{
	std::vector< std::vector< unsigned int > > polygons;
	std::string ext = ToLower( GetFileExtension( fileName ) );
	if( ext==std::string( "ply" ) )
	{
		using namespace VertexFactory;
		using PLYVertexFactory = Factory< Real , PositionFactory< Real , Dim > , NormalFactory< Real , Dim > , ValueFactory< Real > , RGBColorFactory< Real > >;
		using PLYVertex = typename PLYVertexFactory::VertexType;

		std::vector< PLYVertex > plyVertices;
		bool hasNormals , hasValues , hasColors;

		{
			PLYVertexFactory factory;
			std::vector< bool > flags;

			PLY::ReadPolygons( fileName , factory , plyVertices , polygons , flags );
			if( !factory.template plyValidReadProperties< 0 >( flags ) ) MK_THROW( "Vertices do not have positions" );
			hasNormals = factory.template plyValidReadProperties< 1 >( flags );
			hasValues = factory.template plyValidReadProperties< 2 >( flags );
			hasColors = factory.template plyValidReadProperties< 3 >( flags );
		}

		vertices.resize( plyVertices.size() );
		normals.resize( hasNormals ? plyVertices.size() : 0 );
		values.resize( hasValues ? plyVertices.size() : 0 );
		colors.resize( hasColors ? plyVertices.size() : 0 );
		for( unsigned int i=0 ; i<plyVertices.size() ; i++ )
		{
			vertices[i] = plyVertices[i].template get<0>();
			if( hasNormals ) normals[i] = plyVertices[i].template get<1>();
			if( hasValues ) values[i] = plyVertices[i].template get<2>();
			if( hasColors ) colors[i] = plyVertices[i].template get<3>();
		}
	}
	else if( ext==std::string( "obj" ) )
	{
		std::vector< Point< Real , Dim > > obj_vertices;
		std::vector< Point< Real , Dim > > obj_normals;
		std::vector< Real > obj_values;
		std::vector< std::vector< int > > obj_faces;
		std::ifstream in( fileName );
		if( !in.is_open() ) MK_THROW( "Could not open file for reading: " , fileName );

		std::string( line );
		while( std::getline( in , line ) )
		{
			std::stringstream ss;

			// Read vertex position
			if( line[0]=='v' && line[1]==' ' )
			{
				line = line.substr(2);
				std::stringstream ss( line );
				Point< Real , Dim > p;
				for( unsigned int d=0 ; d<Dim ; d++ ) ss >> p[d];
				obj_vertices.push_back( p );
			}
			// Read normal
			else if( line[0]=='v' && line[1]=='n' && line[2]==' ' )
			{
				line = line.substr(3);
				std::stringstream ss( line );
				Point< Real , Dim > p;
				for( unsigned int d=0 ; d<Dim ; d++ ) ss >> p[d];
				obj_normals.push_back( p );
			}
			// Read parameter
			else if( line[0]=='v' && line[1]=='p' && line[2]==' ' )
			{
				line = line.substr(3);
				std::stringstream ss( line );
				Real v;
				ss >> v;
				obj_values.push_back( v );
			}
			// Read face
			else if( line[0]=='f' && line[1]==' ' )
			{
				std::vector< int > face;
				line = line.substr(1);
				while( line.size() && line[0]==' ' ) line = line.substr(1);
				std::stringstream ss( line );
				std::string token;
				while( std::getline( ss , token , ' ' ) )
				{
					std::stringstream _ss(token);
					std::string _token;
					bool firstTime = true;
					int idx;
					while( std::getline( _ss , _token , '/' ) )
					{
						if( firstTime ) idx = std::stoi( _token );
						firstTime = false;
					}
					face.push_back( idx );
				}
				obj_faces.push_back( face );
			}
		}
		if( obj_normals.size() && obj_normals.size()!=obj_vertices.size() ) MK_THROW( "Number of normals does not match number of vertices: " , obj_normals.size() , " != " , obj_vertices.size() );
		if( obj_values.size() && obj_values.size()!=obj_vertices.size() ) MK_THROW( "Number of values does not match number of vertices: " , obj_values.size() , " != " , obj_vertices.size() );


		vertices.resize( obj_vertices.size() );
		for( unsigned int i=0 ; i<obj_vertices.size() ; i++ ) vertices[i] = obj_vertices[i];

		normals.resize( obj_normals.size() );
		for( unsigned int i=0 ; i<obj_normals.size() ; i++ ) normals[i] = obj_normals[i];

		values.resize( obj_values.size() );
		for( unsigned int i=0 ; i<obj_values.size() ; i++ ) values[i] = obj_values[i];

		auto ObjIndexToArrayIndex = [&]( size_t sz , int index ) -> unsigned int
			{
				if( index>0 ) return static_cast< unsigned int >( index-1 );
				else          return static_cast< unsigned int >( sz + index );
			};

		polygons.resize( obj_faces.size() );
		for( unsigned int i=0 ; i<obj_faces.size() ; i++ )
		{
			std::vector< unsigned int > & polygon = polygons[i];
			polygon.resize( obj_faces[i].size() );
			for( unsigned int j=0 ; j<obj_faces[i].size() ; j++ ) polygon[j] = ObjIndexToArrayIndex( obj_vertices.size() , obj_faces[i][j] );
		}
	}
	else MK_THROW( "Unrecognized file type: " , fileName , " -> " , ext );

	// Triangulate the polygons using a minimal area triangulation
	for( unsigned int i=0 ; i<polygons.size() ; i++ )
	{
		const std::vector< unsigned int > & polygon = polygons[i];
		if( polygon.size()>(K+1) )
		{
			std::vector< Point< Real , Dim > > _vertices( polygon.size() );
			std::vector< SimplexIndex< K > > _triangles;
			for( unsigned int j=0 ; j<polygon.size() ; j++ ) _vertices[j] = vertices[ polygon[j] ];
			MinimalAreaTriangulation::GetTriangulation( _vertices , _triangles );

			for( unsigned int j=0 ; j<_triangles.size() ; j++ ) for( unsigned int k=0 ; k<=K ; k++ ) _triangles[j][k] = polygon[ _triangles[j][k] ];
			triangles.insert( triangles.end() , _triangles.begin() , triangles.end() );
		}
		else
		{
			SimplexIndex< K > si;
			for( unsigned int k=0 ; k<=K ; k++ ) si[k] = polygon[k];
			triangles.push_back( si );
		}
	}
}

inline void Mesh::write( std::string fileName ) const
{
	if     ( normals.size() && values.size() ) _write< true  , true  >( fileName );
	else if( normals.size() )                  _write< true  , false >( fileName );
	else if(                   values.size() ) _write< false , true  >( fileName );
	else                                       _write< false , false >( fileName );
}


template< bool HasNormals , bool HasValues >
void Mesh::_write( std::string fileName ) const
{
	std::string ext = ToLower( GetFileExtension( fileName ) );
	if( ext==std::string( "ply" ) )
	{
		using PlyData = _PlyData< HasNormals , HasValues >;
		using VertexFactory = typename PlyData::Factory;
		using Vertex = typename PlyData::Vertex;

		VertexFactory factory;

		std::vector< Vertex > plyVertices( vertices.size() );
		for( unsigned int i=0 ; i<vertices.size() ; i++ )
		{
			PlyData::SetPosition( plyVertices[i] , vertices[i] );
			if constexpr( HasNormals ) PlyData::SetNormal( plyVertices[i] , normals[i] );
			if constexpr( HasValues ) PlyData::SetValue( plyVertices[i] , values[i] );
		}

		PLY::WriteSimplices( fileName , factory , plyVertices , triangles , PLY_BINARY_NATIVE  );
	}
	else if( ext==std::string( "obj" ) )
	{
		std::ofstream out( fileName );
		if( !out.is_open() ) MK_THROW( "Could not open file for writing: " , fileName );

		for( size_t i=0 ; i<vertices.size() ; i++ )
		{
			out << "v";
			for( unsigned int d=0 ; d<Dim ; d++ ) out << " " << vertices[i][d];
			out << std::endl;
		}

		if constexpr( HasNormals )
		{
			for( size_t i=0 ; i<normals.size() ; i++ )
			{
				out << "vt";
				for( unsigned int d=0 ; d<Dim ; d++ ) out << " " << normals[i][d];
				out << std::endl;
			}
		}

		if constexpr( HasValues )
		{
			for( size_t i=0 ; i<values.size() ; i++ )
			{
				out << "vp";
				out << " " << values[i];
				out << std::endl;
			}
		}


		for( size_t i=0 ; i<triangles.size() ; i++ )
		{
			out << "f";
			for( unsigned int k=0 ; k<=K ; k++ ) out << " " << ( triangles[i][k]+1);
			out << std::endl;
		}
	}
	else MK_THROW( "Unrecognized file type: " , fileName , " -> " , ext );
}

inline void Mesh::validate( void ) const
{
	if( normals.size() && normals.size()!=vertices.size() ) MK_THROW( "Normal count does not match vertex count: " , normals.size() , " != " , vertices.size() );
	if( values.size() && values.size()!=vertices.size() ) MK_THROW( "Value count does not match vertex count: " , values.size() , " != " , vertices.size() );
	for( unsigned int t=0 ; t<triangles.size() ; t++ ) for( unsigned int k=0 ; k<=K ; k++ ) if( triangles[t][k]>=vertices.size() )
		MK_THROW( "Triangle vertex index exceeds number of vertices[" , t , " ][" , k , ": " , triangles[t][k] , " >= " , vertices.size() );

}

//////////////////////////////////////
// Mesh::_PlyData (specializations) //
//////////////////////////////////////

template<>
struct Mesh::_PlyData< true , true >
{
	using Factory = VertexFactory::Factory< Real , VertexFactory::PositionFactory< Real , Dim > , VertexFactory::NormalFactory< Real , Dim > , VertexFactory::ValueFactory< Real > >;
	using Vertex = typename Factory::VertexType;
	static void SetPosition( Vertex & vertex , Point< Real , Dim > p ){ vertex.template get<0>() = p; }
	static void SetNormal( Vertex & vertex , Point< Real , Dim > n ){ vertex.template get<1>() = n; }
	static void SetValue( Vertex & vertex , Real v ){ vertex.template get<2>() = v; }
};
template<>
struct Mesh::_PlyData< true , false >
{
	using Factory = VertexFactory::Factory< Real , VertexFactory::PositionFactory< Real , Dim > , VertexFactory::NormalFactory< Real , Dim > >;
	using Vertex = typename Factory::VertexType;
	static void SetPosition( Vertex & vertex , Point< Real , Dim > p ){ vertex.template get<0>() = p; }
	static void SetNormal( Vertex & vertex , Point< Real , Dim > n ){ vertex.template get<1>() = n; }
	static void SetValue( Vertex & vertex , Real v ){}
};
template<>
struct Mesh::_PlyData< false , true >
{
	using Factory = VertexFactory::Factory< Real , VertexFactory::PositionFactory< Real , Dim > , VertexFactory::ValueFactory< Real > >;
	using Vertex = typename Factory::VertexType;
	static void SetPosition( Vertex & vertex , Point< Real , Dim > p ){ vertex.template get<0>() = p; }
	static void SetNormal( Vertex & vertex , Point< Real , Dim > n ){}
	static void SetValue( Vertex & vertex , Real v ){ vertex.template get<1>() = v; }
};
template<>
struct Mesh::_PlyData< false , false >
{
	using Factory = VertexFactory::PositionFactory< Real , Dim >;
	using Vertex = typename Factory::VertexType;
	static void SetPosition( Vertex & vertex , Point< Real , Dim > p ){ vertex = p; }
	static void SetNormal( Vertex & vertex , Point< Real , Dim > n ){}
	static void SetValue( Vertex & vertex , Real v ){}
};

/////////////////////////////
// Mesh::_EdgeInfo::Hasher //
/////////////////////////////
inline size_t Mesh::_EdgeInfo::Hasher::operator()( const std::pair< unsigned int , unsigned int > & e ) const { return e.first; }

/////////////////////
// Mesh::_EdgeInfo //
/////////////////////
inline Mesh::_EdgeInfo::_EdgeInfo( void ){}

inline void Mesh::_EdgeInfo::set( const std::vector< SimplexIndex< K > > & triangles )
{
	unsigned int eIndex = 0;
	for( size_t t=0 ; t<triangles.size() ; t++ )
	{
		for( unsigned int k=0 ; k<=K ; k++ ) for( unsigned int l=0 ; l<k ; l++ )
		{
			std::pair< unsigned int , unsigned int > endPoints = triangles[t][k]<triangles[t][l] ? std::make_pair( triangles[t][k] , triangles[t][l] ) : std::make_pair( triangles[t][l] , triangles[t][k] );
			if( edgeToIndex.find( endPoints )==edgeToIndex.end() ) edgeToIndex[ endPoints ] = eIndex++;
		}
	}
	indexToEdge.resize( eIndex );
	for( auto iter=edgeToIndex.begin() ; iter!=edgeToIndex.end() ; iter++ ) indexToEdge[ iter->second ] = iter->first;
}

