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

#include <Misha/CmdLineParser.h>
#include "PreProcessing.h"
#include "DynamicMeshViewer.h"
#include "RiemannianMesh.h"

using namespace MishaK;
using namespace MishaK::AdvancedGraphics;

CmdLineParameter< std::string >
	In( "in" ) , 
	Transform( "xForm" );

CmdLineParameter< unsigned int >
	Width( "width" , 640 ) ,
	Height( "height" , 480 );

CmdLineParameter< double >
	StepSize( "stepSize" , 1e-4 );

CmdLineReadable
	GouraudShading( "gouraud" ) ,
	UpdateMass( "updateMass" ) ,
	UpdateStiffness( "updateStiffness" ) ,
	Normalize( "normalize" );

std::vector< CmdLineReadable* > params =
{
	&In ,
	&Transform ,
	&Width ,
	&Height ,
	&StepSize ,
	&GouraudShading ,
	&UpdateMass ,
	&UpdateStiffness ,
	&Normalize ,
};

void ShowUsage( std::string ex )
{
	std::cout << "Usage: " << ex << std::endl;
	std::cout << "\t --" << In.name << " <input geometry>" << std::endl;
	std::cout << "\t[--" << Transform.name << " <transform>]" << std::endl;
	std::cout << "\t[--" << Width.name << " <width> = " << Width.value << "]" << std::endl;
	std::cout << "\t[--" << Height.name << " <height> = " << Height.value << "]" << std::endl;
	std::cout << "\t[--" << StepSize.name << " <step-size> = " << StepSize.value << "]" << std::endl;
	std::cout << "\t[--" << GouraudShading.name << "]" << std::endl;
	std::cout << "\t[--" << UpdateMass.name << "]" << std::endl;
	std::cout << "\t[--" << UpdateStiffness.name << "]" << std::endl;
	std::cout << "\t[--" << Normalize.name << "]" << std::endl;
}

struct LaplacianSmoothingViewer : public DynamicMeshViewer
{
	// The strength of the source at selected vertices (for signal averaging)
	double stepSize;
	bool updateMass , updateStiffness;

	LaplacianSmoothingViewer( Mesh & mesh , double stepSize , DynamicMeshViewer::Parameters parameters , bool normalize )
		: DynamicMeshViewer( mesh , parameters )
		, _mesh(mesh)
		, stepSize(stepSize)
		, updateMass(false)
		, updateStiffness(false)
		, _normalize(normalize)
	{
		_stepSizeIndex = static_cast< unsigned int >( info.size() );
		info.resize( info.size()+1 );
		info.back() = std::string( "Step-size:" ) + std::to_string( stepSize );

		_mass = RiemannianMesh::ScalarMass( _mesh );
		_stiffness = RiemannianMesh::ScalarStiffness( _mesh );

		Miscellany::PerformanceMeter pMeter( '.' );

		////////////////////////////////////////////
		// [IMPLEMENT SOLVER INITIALIZATION HERE] //
		MK_WARN_ONCE( "Solver not initialized" );
		////////////////////////////////////////////

		if( _solver.info()!=Eigen::Success ) MK_ERROR_OUT( "Failed to factorize matrix" );
		std::cout << pMeter( "Factorized" ) << std::endl;

		addCallBack( ']' , "increase step-size" , &LaplacianSmoothingViewer::_increaseStepSizeCallBack );
		addCallBack( '[' , "decrease step-size" , &LaplacianSmoothingViewer::_decreaseStepSizeCallBack );
	}

	// Performs the averaging of the geometry/signal
	void animate( void )
	{
		{
			if( updateMass || updateStiffness )
			{
				if( updateMass ) _mass = RiemannianMesh::ScalarMass( _mesh );
				if( updateStiffness ) _stiffness = RiemannianMesh::ScalarStiffness( _mesh );
				_updateNumericalFactorization();
				if( _solver.info()!=Eigen::Success ) MK_ERROR_OUT( "Failed to factorize matrix" );
			}

			Eigen::VectorXd x( _mesh.vertices.size() );
			for( unsigned int d=0 ; d<3 ; d++ )
			{
				for( unsigned int i=0 ; i<_mesh.vertices.size() ; i++ ) x[i] = _mesh.vertices[i][d];
				x = _solver.solve( _mass * x );
				for( unsigned int i=0 ; i<_mesh.vertices.size() ; i++ ) _mesh.vertices[i][d] = x[i];
			}
		}
		if( _normalize ) _mesh.normalize();
		visualizationNeedsUpdating();
	}

protected:
	bool _normalize;
	unsigned int _stepSizeIndex;
	Mesh & _mesh;
	Eigen::SparseMatrix< double > _mass , _stiffness;
	LDLtSolver _solver;

	void _decreaseStepSizeCallBack( std::string )
	{
		stepSize /= 1.1;
		info[ _stepSizeIndex ] = std::string( "Step-size:" ) + std::to_string( stepSize );
		_updateNumericalFactorization();
	}

	void _increaseStepSizeCallBack( std::string )
	{
		stepSize *= 1.1;
		info[ _stepSizeIndex ] = std::string( "Step-size:" ) + std::to_string( stepSize );
		_updateNumericalFactorization();
	}

	void _updateNumericalFactorization( void )
	{

		/////////////////////////////////////////////////////
		// [IMPLEMENT NUMERICAL_FACTORIZATION UPDATE HERE] //
		MK_WARN_ONCE( "Numerical factorization update not implemented" );
		/////////////////////////////////////////////////////

	}
};

int main( int argc , char* argv[] )
{
	CmdLineParse( argc-1 , argv+1 , params );
	if( !In.set )
	{
		ShowUsage( argv[0] );
		return EXIT_FAILURE;
	}

	DynamicMeshViewer::OutputMouseInterfaceControls( std::cout , true );

	Mesh mesh( In.value );

	// Set the colors from the normals
	{
		std::vector< Point< double , 3 > > normals( mesh.vertices.size() );
		for( size_t i=0 ; i<mesh.triangles.size() ; i++ )
		{
			Point< double , 3 > v[] = { mesh.vertices[ mesh.triangles[i][0] ] , mesh.vertices[ mesh.triangles[i][1] ] , mesh.vertices[ mesh.triangles[i][2] ] };
			Point< double , 3 > n = Point< double , 3 >::CrossProduct( v[1]-v[0] , v[2]-v[0] );
			for( unsigned int k=0 ; k<3 ; k++ ) normals[ mesh.triangles[i][k] ] += n;
		}
		for( size_t i=0 ; i<normals.size() ; i++ ) normals[i] /= Point< double , 3 >::Length( normals[i] );

		mesh.colors.resize( mesh.vertices.size() );
		for( size_t i=0 ; i<normals.size() ; i++ ) mesh.colors[i] = ( normals[i] + Point< double , 3 >( 1. , 1. , 1. ) ) / 2.;
	}

	std::cout << "Vertices / Triangles: " << mesh.vertices.size() << " / " << mesh.triangles.size() << std::endl;

	try{ RiemannianMesh::Validate( mesh ); }
	catch( const Exception & e ){ std::cout << e.what() << std::endl; }

	// So that the results of the comptuation do not depend on the scale, normalize
	mesh.normalize();

	DynamicMeshViewer::Parameters parameters;
	parameters.flatShading = !GouraudShading.set;
	parameters.selectionType = DynamicMeshViewer::SelectionType::NONE;
	LaplacianSmoothingViewer v( mesh , StepSize.value , parameters , Normalize.set );

	v.screenWidth = Width.value;
	v.screenHeight = Height.value;
	v.updateMass = UpdateMass.set;
	v.updateStiffness = UpdateStiffness.set;
	if( Transform.set ) v.readXForm( Transform.value );

	LaplacianSmoothingViewer::Viewer::Run( &v , argc , argv , "Laplacian Smoothing: " + In.value );
	return EXIT_SUCCESS;
}