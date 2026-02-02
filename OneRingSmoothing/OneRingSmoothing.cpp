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

#include "PreProcessing.h"

#include <Misha/CmdLineParser.h>
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
	SourceAmplitude( "amplitude" , 1000. );

CmdLineReadable
	GouraudShading( "gouraud" ) ,
	PointSelection( "pointSelection" ) ,
	UpdateBoundingBox( "updateBBox" ) ,
	SmoothGeometry( "geometry" );

std::vector< CmdLineReadable* > params =
{
	&In ,
	&Transform ,
	&Width ,
	&Height ,
	&SourceAmplitude ,
	&GouraudShading ,
	&PointSelection ,
	&UpdateBoundingBox ,
	&SmoothGeometry
};

void ShowUsage( std::string ex )
{
	std::cout << "Usage: " << ex << std::endl;
	std::cout << "\t --" << In.name << " <input geometry>" << std::endl;
	std::cout << "\t[--" << Transform.name << " <transform>]" << std::endl;
	std::cout << "\t[--" << Width.name << " <width> = " << Width.value << "]" << std::endl;
	std::cout << "\t[--" << Height.name << " <height> = " << Height.value << "]" << std::endl;
	std::cout << "\t[--" << SourceAmplitude.name << " <source ampitude> = " << SourceAmplitude.value << "]" << std::endl;
	std::cout << "\t[--" << PointSelection.name << "]" << std::endl;
	std::cout << "\t[--" << GouraudShading.name << "]" << std::endl;
	std::cout << "\t[--" << UpdateBoundingBox.name << "]" << std::endl;
	std::cout << "\t[--" << SmoothGeometry.name << "]" << std::endl;
}

struct OneRingSmoothingViewer : public DynamicMeshViewer
{
	// The strength of the source at selected vertices (for signal averaging)
	double sourceAmplitude;

	OneRingSmoothingViewer( Mesh & mesh , bool smoothSignal , double sourceAmplitude , DynamicMeshViewer::Parameters parameters )
		: DynamicMeshViewer( mesh , parameters )
		, _smoothSignal(smoothSignal)
		, _mesh(mesh)
		, sourceAmplitude(sourceAmplitude)
	{}

	// Performs the averaging of the geometry/signal
	void animate( void )
	{
		if( _smoothSignal )
		{
			//////////////////////////////////////
			// [IMPLEMENT VALUE SMOOTHING HERE] //
			MK_WARN_ONCE( "Value smoothing not imlpemented" );
			//////////////////////////////////////
		}
		else
		{
			///////////////////////////////////////
			// [IMPLEMENT VERTEX SMOOTHING HERE] //
			MK_WARN_ONCE( "Vertex smoothing not imlpemented" );
			///////////////////////////////////////
		}
		visualizationNeedsUpdating();
	}

	// Processes vertex selection
	void selectLeft( const std::vector< std::pair< unsigned int , double > > &selection )
	{
		if( _smoothSignal )
		{
			for( unsigned int i=0 ; i<selection.size() ; i++ ) _mesh.values[ selection[i].first ] -= selection[i].second;
			visualizationNeedsUpdating();
		}
	}

	void selectRight( const std::vector< std::pair< unsigned int , double > > &selection )
	{
		if( _smoothSignal )
		{
			for( unsigned int i=0 ; i<selection.size() ; i++ ) _mesh.values[ selection[i].first ] += selection[i].second;
			visualizationNeedsUpdating();
		}
	}

	// Processes vertex selection
	void selectLeft( unsigned int vIdx )
	{
		if( _smoothSignal )
		{
			_mesh.values[vIdx] = sourceAmplitude;
			visualizationNeedsUpdating();
		}
	}

	void selectRight( unsigned int vIdx )
	{
		if( _smoothSignal )
		{
			_mesh.values[vIdx] = -sourceAmplitude;
			visualizationNeedsUpdating();
		}
	}

protected:
	bool _smoothSignal;
	Mesh & _mesh;
};

int main( int argc , char* argv[] )
{
	CmdLineParse( argc-1 , argv+1 , params );
	if( !In.set )
	{
		ShowUsage( argv[0] );
		return EXIT_FAILURE;
	}

	DynamicMeshViewer::OutputMouseInterfaceControls( std::cout );
	
	Mesh mesh( In.value );
	bool hasColor = false;
	if( mesh.colors.size() )
	{
		if( mesh.values.size() ) MK_WARN( "Colors and values provided. Using values" );
		else
		{
			mesh.values.resize( mesh.colors.size() );
			for( unsigned int i=0 ; i<mesh.values.size() ; i++ ) mesh.values[i] = Point< double , 3 >::Dot( mesh.colors[i] , Point< double , 3 >( 1./3 , 1./3 , 1./3 ) ) / 255.;
			hasColor = true;
		}
	}
	std::cout << "Vertices / Triangles: " << mesh.vertices.size() << " / " << mesh.triangles.size() << std::endl;

	if( !SmoothGeometry.set && !mesh.values.size() )
	{
		mesh.values.resize( mesh.vertices.size() );
		for( unsigned int i=0 ; i<mesh.vertices.size() ; i++ ) mesh.values[i] = 0;
	}

	DynamicMeshViewer::Parameters parameters;
	parameters.flatShading = !GouraudShading.set;
	parameters.selectionType = SmoothGeometry.set ? DynamicMeshViewer::SelectionType::NONE : ( PointSelection.set ?  DynamicMeshViewer::SelectionType::CLICK : DynamicMeshViewer::SelectionType::DRAG );
	if( hasColor ) parameters.grayScale = true;
	else           parameters.valueNormalizationFunction = []( double v ){ return ( v + 1. ) / 2.; };
	OneRingSmoothingViewer v( mesh , !SmoothGeometry.set , SourceAmplitude.value , parameters );

	v.screenWidth = Width.value;
	v.screenHeight = Height.value;
	v.updateBoundingBox = UpdateBoundingBox.set;
	if( Transform.set ) v.readXForm( Transform.value );

	OneRingSmoothingViewer::Viewer::Run( &v , argc , argv , "One-Ring Smoothing: " + In.value );
	return EXIT_SUCCESS;
}