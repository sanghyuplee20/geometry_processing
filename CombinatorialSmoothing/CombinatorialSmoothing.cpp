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
	Implicit( "implicit" );

std::vector< CmdLineReadable* > params =
{
	&In ,
	&Transform ,
	&Width ,
	&Height ,
	&StepSize ,
	&GouraudShading ,
	&Implicit
};

void ShowUsage( std::string ex )
{
	std::cout << "Usage: " << ex << std::endl;
	std::cout << "\t --" << In.name << " <input geometry>" << std::endl;
	std::cout << "\t[--" << Transform.name << " <transform>]" << std::endl;
	std::cout << "\t[--" << Width.name << " <width> = " << Width.value << "]" << std::endl;
	std::cout << "\t[--" << Height.name << " <height> = " << Height.value << "]" << std::endl;
	std::cout << "\t[--" << StepSize.name << " <gradient descent step size> = " << StepSize.value << "]" << std::endl;
	std::cout << "\t[--" << GouraudShading.name << "]" << std::endl;
	std::cout << "\t[--" << Implicit.name << "]" << std::endl;
}

struct CombinatorialSmoothingViewer : public DynamicMeshViewer
{
	CombinatorialSmoothingViewer( Mesh & mesh , double stepSize , bool implicit , DynamicMeshViewer::Parameters parameters )
		: DynamicMeshViewer( mesh , parameters )
		, _mesh(mesh)
		, _implicit(implicit)
		, _stepSize(stepSize)
		, _Id( mesh.vertices.size() , mesh.vertices.size() )
	{
		_stepSizeIndex = static_cast< unsigned int >( info.size() );
		info.resize( info.size()+1 );
		info.back() = std::string( "Stiffness Weight:" ) + std::to_string( _stepSize );
		_Id.setIdentity();

		// Set the combinatorial Laplacian here
		Miscellany::PerformanceMeter pMeter( '.' );

		////////////////////////////////////////////
		// [IMPLEMENT SOLVER INITIALIZATION HERE] //
		MK_WARN_ONCE( "Solver not initialized" );
		////////////////////////////////////////////

		std::cout << pMeter( "Set up system" ) << std::endl;
	}

	// Performs the averaging of the geometry/signal
	void animate( void )
	{

		////////////////////////////////////////////
		// [IMPLEMENT VERTEX UPDATE HERE] //
		MK_WARN_ONCE( "Vertex update not initialized" );
		////////////////////////////////////////////

		visualizationNeedsUpdating();
	}

protected:
	unsigned int _stepSizeIndex;
	Mesh & _mesh;
	Eigen::SparseMatrix< double > _Id , _L;
	Eigen::SimplicialLDLT< Eigen::SparseMatrix< double > > _solver;
	double _stepSize;
	bool _implicit;
};

int main( int argc , char* argv[] )
{
	CmdLineParse( argc-1 , argv+1 , params );
	if( !In.set )
	{
		ShowUsage( argv[0] );
		return EXIT_FAILURE;
	}

	DynamicMeshViewer::OutputMouseInterfaceControls( std::cout , false );

	Mesh mesh( In.value );
	std::cout << "Vertices / Triangles: " << mesh.vertices.size() << " / " << mesh.triangles.size() << std::endl;

	DynamicMeshViewer::Parameters parameters;
	parameters.flatShading = !GouraudShading.set;
	parameters.selectionType = DynamicMeshViewer::SelectionType::NONE;
	parameters.valueNormalizationFunction = []( double v ){ return ( v + 1. ) / 2.; };
	CombinatorialSmoothingViewer v( mesh , StepSize.value , Implicit.set , parameters );

	v.screenWidth = Width.value;
	v.screenHeight = Height.value;
	if( Transform.set ) v.readXForm( Transform.value );

	CombinatorialSmoothingViewer::Viewer::Run( &v , argc , argv , "Combinatorial Smoothing: " + In.value );
	return EXIT_SUCCESS;
}