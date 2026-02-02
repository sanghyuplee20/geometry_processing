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
#include <functional>
#include <Misha/Miscellany.h>
#include <Misha/Visualization.h>
#include <Misha/PlyVertexData.h>
#include <Misha/Camera.h>
#include "Mesh.h"

namespace MishaK
{
	namespace AdvancedGraphics
	{
		struct DynamicMeshViewer : public Visualization::Viewable< DynamicMeshViewer >
		{
			enum SelectionType
			{
				NONE ,
				CLICK ,
				DRAG
			};
			static const std::vector< std::string > selectionTypeNames;

			// A class for tracking the viewing parameters
			struct Parameters
			{
				Parameters
				(
					SelectionType selectionType=SelectionType::NONE ,
					bool flatShading=true ,
					unsigned int bands=0 ,
					double bandEpsilon=0.1 ,
					bool dualBand=true ,
					bool grayScale=false ,
					std::function< double ( double ) > normalizationFunction = []( double v ){ return v; }
				);

				// The selection model
				SelectionType selectionType;

				// Flat or Gouaraud shading
				bool flatShading;

				// The number of bands across the 1D color texture
				unsigned int bands;

				// The width of each band
				double bandEpsilon;

				// Is the band centered on the integers
				bool dualBand;

				// Show the value as gray-scale
				bool grayScale;

				// The function transforming the range values to the range [0,1]
				std::function< double( double ) > valueNormalizationFunction;
			};

			/////////////////
			// Member data //
			/////////////////

			// The radius of the selection ball
			double sphereSelectionRadius;

			// Time (in seconds) to define a mouse "click"
			double clickTime = 0.25;

			// Light properties
			GLfloat lightAmbient[4] , lightDiffuse[4] , lightSpecular[4] , shapeSpecular[4] , shapeSpecularShininess;

			// Should edges be displayed
			bool showEdges;

			// Should the bounding box used for centering/scaling be re-computed when the mesh is updated
			bool updateBoundingBox;

			// How many steps of animation to perform
			// -1 indicates infinite
			unsigned int animationCount;

			// Default color for rendering the mesh when no values are provided
			Point< double , 3 > defaultColor;

			// Should the values be visualized as colors
			bool showValues;

			//////////////////////////////
			// Virtual member functions //
			//////////////////////////////

			// A function for updating the positions/normals/valus of the mesh
			// -- Returns true if the mesh has been updated
			virtual void animate( void );

			// Function for handling a vertex selection with the left mouse button
			// -- Returns true if the mesh has been updated
			virtual void selectLeft ( unsigned int vIdx );

			// Function for handling a vertex selection with the right mouse button
			// -- Returns true if the mesh has been updated
			virtual void selectRight( unsigned int vIdx );

			// Function for handling a vertex drag with the left mouse button
			// -- Returns true if the mesh has been updated
			virtual void dragLeft ( unsigned int vIdx , Point3D< double > p );

			// Function for handling a vertex drag with the right mouse button
			// -- Returns true if the mesh has been updated
			virtual void dragRight( unsigned int vIdx , Point3D< double > p );

			// Function for handling multiple vertex selection with the left mouse button
			// -- Returns true if the mesh has been updated
			virtual void selectLeft ( const std::vector< std::pair< unsigned int , double > > &selection );

			// Function for handling multiple a vertex selection with the right mouse button
			// -- Returns true if the mesh has been updated
			virtual void selectRight( const std::vector< std::pair< unsigned int , double > > &selection );

			// Functions that can be overriden to process keyboard events
			virtual void keyboardFunc( unsigned char key , int x , int y );
			virtual void specialFunc( int key , int x , int y );


			//////////////////////
			// Member functions //
			//////////////////////

			// Constructor/desctructor
			DynamicMeshViewer( const Mesh & mesh , Parameters parameters );
			~DynamicMeshViewer( void ){ delete[] _vboBuffer; }

			// Rendering call-back
			void display( void );

			// Idle call-back
			void idle( void );

			// Mouse interaction call-backs
			void mouseFunc( int button , int state , int x , int y );
			void motionFunc( int x , int y );
			void passiveMotionFunc( int x , int y );

			// Functionality for camera transform I/O
			bool writeXForm( std::string fileName ) const;
			bool  readXForm( std::string fileName );

			// Prints the mouse interface controls
			static void OutputMouseInterfaceControls( std::ostream & out );

			// Sets the mapping from values in [-1,1] to colors
			void setColorMap( unsigned int res , unsigned int bands , double bandEpsilon , bool dual , bool useGrayScale );

			// Sets the selection type
			void setSelectionType( SelectionType selectionType );

			// A function to be invoked when the mesh has been updated
			void visualizationNeedsUpdating( void );

		protected:
			const Mesh & _mesh;

			size_t _vNum;
			double * _vboBuffer;

			Parameters _parameters;
			Camera _camera;
			double _zoom;
			Point3D< double > _translate;
			double _scale;
			GLuint _vbo , _ebo;
			int _mouseX , _mouseY;
			bool _rotating , _scaling , _panning;
			bool _dragDiscrete;
			int _vertexIndex;
			int _selectIndex;
			int _animationIndex;
			unsigned int _overVertex;
			Point3D< double > _overPosition;
			Miscellany::Timer _timer;
			double _depressedTime;
			double _boundingRadius;
			bool _selectionMode;
			bool _leftButtonDown , _rightButtonDown;
			unsigned int _colorMapID;
			bool _visualizationNeedsUpdating;

			void _setTranslateAndScale( void );
			Point3D< double > _cameraToWorld( Point3D< double > p , bool direction=false ) const;
			Point3D< double > _worldToCamera( Point3D< double > p , bool direction=false ) const;
			void _setVBOBuffer( bool updateBoundingBox , const std::function< double( double ) > & valueNormalizationFunction );
			void _initVBO( void );
			void _updateVBO( void );

			Point3D< double > _mouseDirection( int dx , int dy ) const;
			std::optional< Point3D< double > > _mousePosition( int x , int y ) const;
			unsigned int _nearestVertex( int x , int y , Point3D< double > & position ) const;

			void _setXFormCallBack( std::string prompt );
			void _toggleEdgesCallBack( std::string );
			void _toggleAnimationCallBack( std::string );
			void _advanceAnimationCallBack( std::string );
			void _decreaseSphereSelectionRadiusCallBack( std::string );
			void _increaseSphereSelectionRadiusCallBack( std::string );
			void _toggleShowValuesCallBack( std::string );
			void _toggleTransformMode( std::string );
			void _drawSelectionSphere( Point3D< double > p , double r );
		};
#include "DynamicMeshViewer.inl"
	}
}