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

const std::vector< std::string > DynamicMeshViewer::selectionTypeNames =
{
	"none" ,
	"click" ,
	"drag"
};

inline DynamicMeshViewer::Parameters::Parameters
(
	SelectionType selectionType ,
	bool flatShading ,
	unsigned int bands ,
	double bandEpsilon ,
	bool dualBand ,
	bool grayScale ,
	std::function< double ( double ) > valueNormalizationFunction
)
	: selectionType(selectionType) , flatShading(flatShading) , bands(bands) , bandEpsilon(bandEpsilon) , dualBand(dualBand) , grayScale(grayScale) , valueNormalizationFunction(valueNormalizationFunction)
{}

inline DynamicMeshViewer::DynamicMeshViewer( const Mesh & mesh , Parameters parameters )
	: _mesh(mesh) , _vboBuffer(nullptr) , _parameters(parameters)
{
	// Check that the mesh is in a reasonable state
	_mesh.validate();

	// Initialize public variables
	animationCount = 0;
	defaultColor = Point3D< double >( 0.75 , 0.75 , 0.75 );
	lightAmbient [0] = lightAmbient [1] = lightAmbient [2] = 0.25f , lightAmbient [3] = 1.f;
	lightDiffuse [0] = lightDiffuse [1] = lightDiffuse [2] = 0.70f , lightDiffuse [3] = 1.f;
	lightSpecular[0] = lightSpecular[1] = lightSpecular[2] = 1.00f , lightSpecular[3] = 1.f;
	shapeSpecular[0] = shapeSpecular[1] = shapeSpecular[2] = 1.00f , shapeSpecular[3] = 1.f;
	shapeSpecularShininess = 128;
	updateBoundingBox = false;
	showEdges = false;
	showValues = true;
	sphereSelectionRadius = 0.2;

	// Initialize private variables
	_zoom = 1.05f;
	_rotating = _scaling = _panning = false;
	_dragDiscrete = false;
	_scale = 1.f;
	_translate = Point3D< double >( 0.f , 0.f , 0.f );
	_overVertex = -1;
	_selectionMode = false;
	_leftButtonDown = _rightButtonDown = false;
	_visualizationNeedsUpdating = false;

	_vbo = _ebo = 0;
	_colorMapID = 0;

	_vNum = _parameters.flatShading ? _mesh.triangles.size() * 3 : _mesh.vertices.size();
	_vboBuffer = new double[ 7 * _vNum ];

	_animationIndex = (int)info.size();
	info.resize( info.size()+1 );
	info[ _animationIndex ] = std::string( "Animation: " ) + ( animationCount==static_cast< unsigned int >(-1) ? std::string( "ON" ) : std::string( "OFF" ) );

	_vertexIndex = (int)info.size();
	info.resize( info.size()+1 );
	info[ _vertexIndex ] = std::string( "Vertex:" );

	if( _parameters.selectionType!=SelectionType::NONE && _parameters.selectionType!=SelectionType::CLICK )
	{
		_selectIndex = (int)info.size();
		info.resize( info.size()+1 );
		info[ _selectIndex ] = std::string( "Select: " ) + ( _selectionMode ? std::string( "ON" ) : std::string( "OFF" ) );
	}

	// Add keyboard call backs
	addCallBack( 'x' , "save transform" , "Transform" , &DynamicMeshViewer::_setXFormCallBack );
	addCallBack( 'e' , "toggle edges"                 , &DynamicMeshViewer::_toggleEdgesCallBack );
	addCallBack( '+' , "advance animation"            , &DynamicMeshViewer::_advanceAnimationCallBack );
	addCallBack( ' ' , "toggle animation"             , &DynamicMeshViewer::_toggleAnimationCallBack );
	if( _mesh.values.size() ) addCallBack( 'v' , "toggle values" , &DynamicMeshViewer::_toggleShowValuesCallBack );

	if( _parameters.selectionType==SelectionType::DRAG )
	{
		addCallBack( 's' , "toggle selection mode" , &DynamicMeshViewer::_toggleTransformMode );
		addCallBack( '}' , "increase selection radius" , &DynamicMeshViewer::_increaseSphereSelectionRadiusCallBack );
		addCallBack( '{' , "decrease selection radius" , &DynamicMeshViewer::_decreaseSphereSelectionRadiusCallBack );
	}
}

inline void DynamicMeshViewer::animate( void ){}
inline void DynamicMeshViewer::selectLeft ( unsigned int vIdx ){}
inline void DynamicMeshViewer::selectRight( unsigned int vIdx ){}
inline void DynamicMeshViewer::dragLeft ( unsigned int vIdx , Point3D< double > p ){}
inline void DynamicMeshViewer::dragRight( unsigned int vIdx , Point3D< double > p ){}
inline void DynamicMeshViewer::selectLeft ( const std::vector< std::pair< unsigned int , double > > &selection ){}
inline void DynamicMeshViewer::selectRight( const std::vector< std::pair< unsigned int , double > > &selection ){}

inline void DynamicMeshViewer::_setXFormCallBack( std::string prompt ){ if( prompt.size() ) writeXForm( prompt ); }

inline void DynamicMeshViewer::_toggleEdgesCallBack( std::string ){ showEdges = !showEdges; }

inline void DynamicMeshViewer::_toggleAnimationCallBack( std::string )
{
	if( !animationCount ) animationCount = static_cast< unsigned int >(-1);
	else                  animationCount = 0;
}

inline void DynamicMeshViewer::_advanceAnimationCallBack( std::string ){ animationCount++; }

inline void DynamicMeshViewer::_decreaseSphereSelectionRadiusCallBack( std::string ){ sphereSelectionRadius /= 1.1; }
inline void DynamicMeshViewer::_increaseSphereSelectionRadiusCallBack( std::string ){ sphereSelectionRadius *= 1.1; }
inline void DynamicMeshViewer::_toggleShowValuesCallBack( std::string ){ showValues = !showValues; }
inline void DynamicMeshViewer::_toggleTransformMode( std::string )
{
	_selectionMode = !_selectionMode;
	info[ _selectIndex ] = std::string( "Select: " ) + ( _selectionMode ? std::string( "ON" ) : std::string( "OFF" ) );
}

inline bool DynamicMeshViewer::writeXForm( std::string fileName ) const
{
	std::ofstream out( fileName );
	if( !out ) return false;
	_camera.write( out );
	out << _zoom << std::endl;
	return static_cast< bool >( out );
}

inline bool DynamicMeshViewer::readXForm( std::string fileName )
{
	std::ifstream in( fileName );
	if( !in ) return false;
	_camera.read( in );
	in >> _zoom;
	return static_cast< bool >(in);
}

inline Point3D< double > DynamicMeshViewer::_cameraToWorld( Point3D< double > p , bool direction ) const
{
	if( direction ) return p / _scale;
	else            return p / _scale - _translate;
}
inline Point3D< double > DynamicMeshViewer::_worldToCamera( Point3D< double > p , bool direction ) const
{
	if( direction ) return ( p              ) * _scale;
	else            return ( p + _translate ) * _scale;
}

inline void DynamicMeshViewer::_setTranslateAndScale( void )
{
	Point3D< double > bBox[2];

	bBox[0] = bBox[1] = _mesh.vertices[0];
	for( unsigned int i=0 ; i<_mesh.vertices.size() ; i++ ) for( unsigned int j=0 ; j<3 ; j++ )
		bBox[0][j] = std::min< double >( bBox[0][j] , _mesh.vertices[i][j] ) , bBox[1][j] = std::max< double >( bBox[1][j] , _mesh.vertices[i][j] );

	// Compute the _translation and _scale bringing the mesh into view
	Point3D< double > center = ( bBox[0] + bBox[1] ) / 2;
	double maxXZ = 0.f , maxY = 0.f;
	Point3D< double > d = bBox[1]-center;
	_boundingRadius = Point2D< double >::Length( Point2D< double >( d[0] , d[2] ) );
	maxXZ = std::max< double >( maxXZ , Point2D< double >::Length( Point2D< double >( d[0] , d[2] ) ) );
	maxY  = std::max< double >( maxY  , fabs( d[1] ) );
	_translate = -center;

	// Scale so that the aspect ratio of the model fits the aspect ratio of the window.
	// If the width>height:
	//		window = [ -width/height , width/height ] x[ -1 , 1 ]
	// =>	Scale should be as large as possible so that:
	//		_scale * maxXZ <= width/height and _scale * maxY<= 1
	// =>	_scale <= width/height / maxXZ and _scale <= 1/maxY
	// Else:
	//		window = [ -1 , 1 ] x [ -height/width , height/width ]
	// =>	Scale should be as large as possible so that:
	//		_scale * maxXZ <= 1 and _scale *maxY <= height/width
	// =>	_scale <= 1 / maxXZ and _scale <= height/width/maxY
	double aspectRatio = (double)screenWidth / (double)screenHeight;
	if( aspectRatio>1 ) _scale = std::min< double >( aspectRatio / maxXZ , 1.f / maxY );
	else                _scale = std::min< double >( 1.f / maxXZ , 1.f / aspectRatio / maxY );
}

inline void DynamicMeshViewer::setSelectionType( SelectionType selectionType ){ _parameters.selectionType = selectionType; }

inline void DynamicMeshViewer::setColorMap( unsigned int res , unsigned int bands , double bandEpsilon , bool dual , bool useGrayScale )
{
	unsigned char * colorValues = new unsigned char[ res * 3 ];

	auto IsBand = [&]( unsigned int i , double epsilon )
		{
			if( !bands ) return false;
			double s = static_cast< double >(i)/( res-1 ) * bands;
			s -= floor(s);
			if( dual ) return fabs( s - 0.5 )<epsilon;
			else       return s<epsilon || s>(1.-epsilon);
		};

	unsigned char innerBorder = 224;
	unsigned char outerBorder = 32;
	for( unsigned int i=0 ; i<res ; i++ )
	{
		if( IsBand( i , bandEpsilon ) )
		{
			if( IsBand( i , bandEpsilon/2 ) ) for( unsigned int d=0 ; d<3 ; d++ ) colorValues[3*i+d] = innerBorder;
			else                              for( unsigned int d=0 ; d<3 ; d++ ) colorValues[3*i+d] = outerBorder;
		}
		else
		{
			if( _parameters.grayScale ) for( unsigned int d=0 ; d<3 ; d++ ) colorValues[3*i+d] = static_cast< double >(i)/(res-1) *  255.;
			else
			{
				double c[3];
				Miscellany::TurboValueToRGB( 1. - static_cast< double >(i)/(res-1) * 2. , c );
				for( unsigned int d=0 ; d<3 ; d++ ) colorValues[3*i+d] = static_cast< unsigned char >( c[d] * 255. );
			}
		}
	}

	if( _colorMapID ) glDeleteTextures( 1 , &_colorMapID );
	_colorMapID = 0;
	glGenTextures( 1 , &_colorMapID );
	glBindTexture  ( GL_TEXTURE_1D , _colorMapID );
	if( !dual )
	{
		GLfloat border[] = { static_cast< GLfloat >( innerBorder ) , static_cast< GLfloat >( innerBorder ) , static_cast< GLfloat >( innerBorder ) , 255 };
		glTexParameterfv( GL_TEXTURE_1D , GL_TEXTURE_BORDER_COLOR , border );
	}
	glTexParameteri( GL_TEXTURE_1D , GL_TEXTURE_WRAP_S , GL_CLAMP );
	glTexParameteri( GL_TEXTURE_1D , GL_TEXTURE_MAG_FILTER , GL_LINEAR );
	glTexParameteri( GL_TEXTURE_1D , GL_TEXTURE_MIN_FILTER , GL_LINEAR );
	glTexImage1D   ( GL_TEXTURE_1D , 0 , GL_RGBA , res , 0 , GL_RGB , GL_UNSIGNED_BYTE , (GLvoid*)colorValues );

	delete[] colorValues;
}

inline void DynamicMeshViewer::_setVBOBuffer( bool updateBoundingBox , const std::function< double ( double ) > & valueNormalizationFunction )
{
	if( updateBoundingBox ) _setTranslateAndScale();

	memset( _vboBuffer , 0 , sizeof(double) * 7 * _vNum );

	Point3D< double > * vertices = reinterpret_cast< Point3D< double >* >( _vboBuffer + 0 * _vNum );
	Point3D< double > * normals  = reinterpret_cast< Point3D< double >* >( _vboBuffer + 3 * _vNum );
	double            * tCoords  =                                         _vboBuffer + 6 * _vNum;

	auto TriangleNormal = [&]( size_t t )
		{
			return Point3D< double >::CrossProduct( _mesh.vertices[ _mesh.triangles[t][1] ] - _mesh.vertices[ _mesh.triangles[t][0] ] , _mesh.vertices[ _mesh.triangles[t][2] ] - _mesh.vertices[ _mesh.triangles[t][0] ] );
		};

	if( _parameters.flatShading )
	{
		if( !_mesh.normals.size() )
		{
			for( unsigned int i=0 , idx=0 ; i<_mesh.triangles.size() ; i++ )
			{
				Point3D< double > n = TriangleNormal(i);
				n /= Point3D< double >::Length( n );

				for( unsigned int j=0 ; j<3 ; j++ , idx++ )	normals [idx] = n;
			}
		}
		else
		{
			for( unsigned int i=0 , idx=0 ; i<_mesh.triangles.size() ; i++ )
			{
				Point3D< double > n;
				for( unsigned int j=0 ; j<3 ; j++ ) n += _mesh.normals[ _mesh.triangles[i][j] ];
				n /= Point3D< double >::Length( n );
				for( unsigned int j=0 ; j<3 ; j++ , idx++ )	normals[idx] = n;
			}
		}

		if( _mesh.values.size() )
		{
			for( unsigned int i=0 , idx=0 ; i<_mesh.triangles.size() ; i++ )
			{
				double v = 0;
				for( unsigned int j=0 ; j<3 ; j++ ) v += _mesh.values[ _mesh.triangles[i][j] ];
				v /= 3.;
				for( unsigned int j=0 ; j<3 ; j++ , idx++ ) tCoords[idx] = valueNormalizationFunction(v);
			}
		}

		for( unsigned int i=0 , idx=0 ; i<_mesh.triangles.size() ; i++ ) for( unsigned int j=0 ; j<3 ; j++ , idx++ )
			vertices[idx] = _worldToCamera( _mesh.vertices[ _mesh.triangles[i][j] ] );
	}
	else
	{
		if( !_mesh.normals.size() )
			for( unsigned int i=0 ; i<_mesh.triangles.size() ; i++ )
			{
				Point3D< double > n = TriangleNormal(i);
				for( unsigned int j=0 ; j<3 ; j++ ) normals[ _mesh.triangles[i][j] ] += n;
			}
		else for( unsigned int i=0 ; i<_mesh.vertices.size() ; i++ ) normals[i] = _mesh.normals[i];

		for( unsigned int i=0 ; i<_mesh.vertices.size() ; i++ )
		{
			vertices[i] = _worldToCamera( _mesh.vertices[i] );
			normals[i] /= Point3D< double >::Length( normals[i] );
			if( _mesh.values.size() ) tCoords[i] = valueNormalizationFunction( _mesh.values[i] );
		}
	}
}

inline void DynamicMeshViewer::_initVBO( void )
{
	_setVBOBuffer( true , _parameters.valueNormalizationFunction );

	std::vector< SimplexIndex< 2 > > triangles( _mesh.triangles.size() );

	if( _parameters.flatShading ) for( unsigned int i=0 , idx=0 ; i<triangles.size() ; i++ ) for( unsigned int j=0 ; j<3 ; j++ , idx++ ) triangles[i][j] = idx;
	else triangles = _mesh.triangles;

	glGenBuffers( 1 , &_ebo );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER , _ebo );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER , triangles.size() * sizeof( unsigned int ) * 3 , &triangles[0] , GL_STATIC_DRAW );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER , 0 );

	glGenBuffers( 1 , &_vbo );
	glBindBuffer( GL_ARRAY_BUFFER , _vbo );
	glBufferData( GL_ARRAY_BUFFER , 7 * _vNum * sizeof( double ) , _vboBuffer , GL_DYNAMIC_DRAW );
	glBindBuffer( GL_ARRAY_BUFFER , 0 );
}

inline void DynamicMeshViewer::visualizationNeedsUpdating( void )
{
	_visualizationNeedsUpdating = true;
}

inline void DynamicMeshViewer::_updateVBO( void )
{
	_setVBOBuffer( updateBoundingBox , _parameters.valueNormalizationFunction );
	glBindBuffer( GL_ARRAY_BUFFER , _vbo );
	glBufferSubData( GL_ARRAY_BUFFER , 0 , 7 * _vNum * sizeof( double ) , _vboBuffer );
	glBindBuffer( GL_ARRAY_BUFFER , 0 );
}

inline Point3D< double > DynamicMeshViewer::_mouseDirection( int dx , int  dy ) const
{
	double ar = (double)screenWidth/(double)screenHeight ;
	double _width , _height;
	if( screenWidth>screenHeight ) _width = screenWidth * ar , _height = screenHeight;
	else                           _width = screenWidth , _height = screenHeight / ar;

	double _x =(double)dx/screenWidth , _y = - (double)dy/screenHeight;
	if( screenWidth>screenHeight ) _x *= _zoom*ar , _y *= _zoom;
	else                           _x *= _zoom , _y *= _zoom/ar;
	_x *= 2. , _y *= 2;
	return _cameraToWorld( _camera.right * _x + _camera.up * _y  , true );
}

inline std::optional< Point3D< double > > DynamicMeshViewer::_mousePosition( int x , int  y ) const
{
	float depth;
	glReadPixels( x , screenHeight-1-y , 1 , 1 , GL_DEPTH_COMPONENT , GL_FLOAT , &depth );
	double ar = (double)screenWidth/(double)screenHeight ;
	double _width , _height;
	if( screenWidth>screenHeight ) _width = screenWidth * ar , _height = screenHeight;
	else                           _width = screenWidth , _height = screenHeight / ar;

	{
		double _x =(double)x/screenWidth - 0.5 , _y = 1. - (double)y/screenHeight - 0.5;
		if( screenWidth>screenHeight ) _x *= _zoom*ar , _y *= _zoom;
		else                           _x *= _zoom , _y *= _zoom/ar;
		_x *= 2. , _y *= 2;
		if( depth<1 ) return _cameraToWorld( _camera.forward * ( -1.5 + 3. * depth ) + _camera.right * _x + _camera.up * _y + _camera.position );
		else return {};
	}
}

inline unsigned int DynamicMeshViewer::_nearestVertex( int x , int y , Point3D< double > & p ) const
{
	if( auto _p=_mousePosition(x,y) )
	{
		p = *_p;
		unsigned int vIdx = static_cast< unsigned int >(-1);
		double l2 = std::numeric_limits< double >::infinity();
		for( unsigned int i=0 ; i<_mesh.vertices.size() ; i++ )
		{
			double _l2 = Point3D< double >::SquareNorm( p - _mesh.vertices[i] );
			if( _l2<l2 ) vIdx = i , l2 = _l2;
		}
		return vIdx;
	}
	else return static_cast< unsigned int >(-1);
}

inline void DynamicMeshViewer::display( void )
{
	bool useTexture = _mesh.values.size()!=0 && showValues;

	if( !_vbo && !_ebo ) _initVBO();
	if( !_colorMapID ) setColorMap( 1024 , _parameters.bands , _parameters.bandEpsilon , _parameters.dualBand , _parameters.grayScale );
	if( _visualizationNeedsUpdating ) _updateVBO() , _visualizationNeedsUpdating = false;

	glDisable( GL_CULL_FACE );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	double ar = (double)screenWidth/(double)screenHeight , ar_r = 1.f/ar;
	if( screenWidth>screenHeight ) glOrtho( -ar*_zoom , ar*_zoom , -_zoom , _zoom , -1.5 , 1.5 );
	else                           glOrtho( -_zoom , _zoom , -ar_r*_zoom , ar_r*_zoom , -1.5 , 1.5 );

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	_camera.draw();

	// Lights
	GLfloat lPosition[4];
	{
		Point3D< double > d = _camera.up + _camera.right - _camera.forward*5;
		lPosition[0] = (float)d[0] , lPosition[1] = (float)d[1] , lPosition[2] = (float)d[2];
	}
	lPosition[3] = 0.0f;	
	glLightModeli( GL_LIGHT_MODEL_LOCAL_VIEWER , GL_FALSE );
	glLightModeli( GL_LIGHT_MODEL_TWO_SIDE , GL_TRUE );
	glLightfv( GL_LIGHT0 , GL_AMBIENT , lightAmbient );
	glLightfv( GL_LIGHT0 , GL_DIFFUSE , lightDiffuse );
	glLightfv( GL_LIGHT0 , GL_SPECULAR , lightSpecular );
	glLightfv( GL_LIGHT0 , GL_POSITION , lPosition );
	glEnable( GL_LIGHT0 );
	glEnable( GL_LIGHTING );

	glColor3f( 0.75f , 0.75f , 0.75f );

	if( useTexture )
	{
		glEnable( GL_TEXTURE_1D );
		glBindTexture( GL_TEXTURE_1D , _colorMapID );
		if( _parameters.grayScale ) glTexEnvi( GL_TEXTURE_ENV , GL_TEXTURE_ENV_MODE , GL_DECAL );
		else                        glTexEnvi( GL_TEXTURE_ENV , GL_TEXTURE_ENV_MODE , GL_MODULATE );
	}

	glEnable( GL_DEPTH_TEST );
	glMaterialfv( GL_FRONT_AND_BACK , GL_SPECULAR  , shapeSpecular );
	glMaterialf ( GL_FRONT_AND_BACK , GL_SHININESS , shapeSpecularShininess );
	glColorMaterial( GL_FRONT_AND_BACK , GL_AMBIENT_AND_DIFFUSE );

	glBindBuffer( GL_ARRAY_BUFFER , _vbo );
	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_NORMAL_ARRAY );
	if( useTexture ) glEnableClientState( GL_TEXTURE_COORD_ARRAY );
	glVertexPointer( 3 , GL_DOUBLE , 0 , (GLubyte*)NULL + sizeof( double ) * _vNum*0 );
	glNormalPointer(     GL_DOUBLE , 0 , (GLubyte*)NULL + sizeof( double ) * _vNum*3 );
	if( useTexture ) glTexCoordPointer( 1 , GL_DOUBLE , 0 , (GLubyte*)NULL + sizeof( double ) * _vNum*6 );

	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER , _ebo );
	glDrawElements( GL_TRIANGLES , (GLsizei)(_mesh.triangles.size()*3) , GL_UNSIGNED_INT , NULL );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER , 0 );

	glDisableClientState( GL_NORMAL_ARRAY );
	if( useTexture )
	{
		glDisableClientState( GL_TEXTURE_COORD_ARRAY );
		glDisable( GL_TEXTURE_1D );
	}
	glBindBuffer( GL_ARRAY_BUFFER , 0 );

	if( showEdges )
	{
		GLint src , dst;
		glGetIntegerv( GL_BLEND_SRC , &src );
		glGetIntegerv( GL_BLEND_DST , &dst );
		Point3D< double > f = _camera.forward / 256;
		glDisable( GL_LIGHTING );
		glPushMatrix();
		{
			glTranslated( -f[0] , -f[1] , -f[2] );
			glColor3f( 0.125 , 0.125 , 0.125 );
			glBlendFunc( GL_SRC_ALPHA , GL_ONE_MINUS_SRC_ALPHA );
			glEnable( GL_BLEND );
			glEnable( GL_LINE_SMOOTH );
			glLineWidth( 0.25f );
			glPolygonMode( GL_FRONT_AND_BACK , GL_LINE );
			glBindBuffer( GL_ARRAY_BUFFER , _vbo );
			glBindBuffer( GL_ELEMENT_ARRAY_BUFFER , _ebo );
			glEnableClientState( GL_VERTEX_ARRAY );
			glVertexPointer( 3 , GL_DOUBLE , 0 , NULL );
			glDrawElements( GL_TRIANGLES , (GLsizei)(_mesh.triangles.size()*3) , GL_UNSIGNED_INT , NULL );
			glBindBuffer( GL_ARRAY_BUFFER , 0 );
			glBindBuffer( GL_ELEMENT_ARRAY_BUFFER , 0 );
			glPolygonMode( GL_FRONT_AND_BACK , GL_FILL );
			glDisable( GL_LINE_SMOOTH );
			glDisable( GL_BLEND );
			glBlendFunc( src , dst );
		}
		glPopMatrix();
	}

	if( _overVertex<_mesh.vertices.size() )
	{
		glDisable( GL_LIGHTING );
		if( _selectionMode && _parameters.selectionType==SelectionType::DRAG && !_dragDiscrete )
			_drawSelectionSphere( _overPosition , sphereSelectionRadius * _boundingRadius );
		else
		{
			glPointSize( 5 );
			glColor3f( 0.f , 0.f , 0.f );
			glBegin( GL_POINTS );
			Point3D< double > p = _worldToCamera( _mesh.vertices[_overVertex] );
			p -= _camera.forward/100;
			glVertex3d( p[0] , p[1] , p[2] );
			glEnd();
		}
	}
}

inline void DynamicMeshViewer::_drawSelectionSphere( Point3D< double > p , double r )
{
	const unsigned int ThetaRes = 128 , PhiRes = 64;
	auto DrawSpherePoint = [&]( unsigned int x , unsigned int y )
		{
			double theta = ( 2. * M_PI * x ) / ThetaRes;
			double phi = ( M_PI * y ) / PhiRes;
			Point3D< double > v = _worldToCamera( p + Point3D< double >( cos( theta ) * sin( phi ) , cos( phi ) , sin( theta ) * sin( phi ) ) * r );
			glVertex3d( v[0] , v[1] , v[2] );
		};

	glBlendFunc( GL_SRC_ALPHA , GL_ONE_MINUS_SRC_ALPHA );
	glEnable( GL_BLEND );
	glColor4d( 0.5 , 0.5 , 0.5 , 0.25 );
	glDepthMask( GL_FALSE );
	glBegin( GL_TRIANGLES );
	for( unsigned int j=1 ; j<PhiRes-1 ; j++ )
	{
		for( unsigned int i=0 ; i<2*ThetaRes ; i++ )
		{
			DrawSpherePoint(i,j);
			DrawSpherePoint(i+1,j);
			DrawSpherePoint(i+1,j+1);

			DrawSpherePoint(i+1,j+1);
			DrawSpherePoint(i,j+1);
			DrawSpherePoint(i,j);
		}
	}
	glEnd();
	glDisable( GL_BLEND );
	glDepthMask( GL_TRUE );
}

inline void DynamicMeshViewer::mouseFunc( int button , int state , int x , int y )
{
	_rotating = _scaling = _panning = false;

	if( state==GLUT_DOWN )
	{
		if( button==GLUT_LEFT_BUTTON  )  _leftButtonDown = true;
		if( button==GLUT_RIGHT_BUTTON ) _rightButtonDown = true;
	}
	else
	{
		if( button==GLUT_LEFT_BUTTON  )  _leftButtonDown = false;
		if( button==GLUT_RIGHT_BUTTON ) _rightButtonDown = false;
	}

	auto ProcessCameraTransformation = [&]( void )
		{
			if( state==GLUT_DOWN )
			{
				if( button==GLUT_LEFT_BUTTON )
				{
					if( glutGetModifiers() & GLUT_ACTIVE_CTRL ) _panning  = true;
					else                                        _rotating = true;
				}
				else if( button==GLUT_RIGHT_BUTTON )            _scaling = true;
			}
		};

	if( state==GLUT_DOWN )
	{
		_mouseX = x , _mouseY = y;
		_depressedTime = _timer.elapsed();
		_overVertex = _nearestVertex( x , y , _overPosition );
	}

	if( _parameters.selectionType==SelectionType::CLICK )
	{
		if( state==GLUT_UP && ( _timer.elapsed() - _depressedTime )<clickTime )
		{
			if( _overVertex!=static_cast< unsigned int >(-1) )
			{
				if( button==GLUT_LEFT_BUTTON  )  selectLeft( _overVertex );
				if( button==GLUT_RIGHT_BUTTON ) selectRight( _overVertex );
			}
		}
		else ProcessCameraTransformation();
		return;
	}

	if( !_selectionMode || _overVertex==static_cast< unsigned int >(-1) ) ProcessCameraTransformation();
	else
	{
		if( state==GLUT_DOWN ) _dragDiscrete = ( glutGetModifiers() & GLUT_ACTIVE_CTRL ) != 0;
		else if( state==GLUT_UP )
			if( _dragDiscrete )
			{
				Point3D< double > p = _mesh.vertices[_overVertex] + _mouseDirection( x - _mouseX , y - _mouseY );
				if( button==GLUT_LEFT_BUTTON  ) dragLeft ( _overVertex , p );
				if( button==GLUT_RIGHT_BUTTON ) dragRight( _overVertex , p );
				_dragDiscrete = false;
			}
	}
}

inline void DynamicMeshViewer::motionFunc( int x , int y )
{
	if( _dragDiscrete ) return;

	int imageSize = std::min< int >( screenWidth , screenHeight );
	double rel_x = (x-_mouseX) / (double)imageSize * 2;
	double rel_y = (y-_mouseY) / (double)imageSize * 2;
	double pRight = -rel_x * _zoom , pUp = rel_y * _zoom;
	double sForward = rel_y*4;
	double rRight = rel_y * _zoom , rUp = rel_x * _zoom;

	_mouseX = x , _mouseY = y;

	if     ( _rotating ) _camera.rotateUp( rUp ) , _camera.rotateRight( rRight );
	else if( _scaling  ) _zoom *= (double)pow( 0.9 , (double)sForward );
	else if( _panning  ) _camera.translate( _camera.right * pRight + _camera.up * pUp );

	glutPostRedisplay();
}

// Tracks the position of the mouse (in 3D) and the index of the nearest vertex
inline void DynamicMeshViewer::passiveMotionFunc( int x , int y )
{
	_overVertex = _nearestVertex( x , y , _overPosition );

	if( _overVertex!=static_cast< unsigned int >(-1) )
	{
		std::stringstream ss;
		Miscellany::StreamFloatPrecision sfp( ss , 3 );
		if( _mesh.values.size() ) ss << "Vertex[" << _overVertex << "]: " << _mesh.vertices[_overVertex] << " / " << _mesh.values[_overVertex];
		else                      ss << "Vertex[" << _overVertex << "]: " << _mesh.vertices[_overVertex];
		info[ _vertexIndex ] = ss.str();
	}
	else info[_vertexIndex] = std::string( "Vertex:" );

	glutPostRedisplay();
}

inline void DynamicMeshViewer::idle( void )
{
	bool redisplay = false;
	if( !promptCallBack )
	{
		if( !_dragDiscrete && !_rotating && !_scaling && !_panning && ( _leftButtonDown || _rightButtonDown ) )
		{
			if( _parameters.selectionType==SelectionType::DRAG )
			{
				double dt = _timer.elapsed() - _depressedTime;
				if( auto p=_mousePosition( _mouseX , _mouseY ) )
				{
					double r = sphereSelectionRadius * _boundingRadius;
					unsigned int vIdx = -1;
					double l2 = std::numeric_limits< double >::infinity();
					std::vector< std::pair< unsigned int , double > > selection;
					for( unsigned int i=0 ; i<_mesh.vertices.size() ; i++ )
					{
						double _l2 = Point3D< double >::SquareNorm( *p - _mesh.vertices[i] );
						if( vIdx==static_cast< unsigned int >(-1) || _l2<l2 ) vIdx = i , l2 = _l2;
						if( _l2<r*r ) selection.emplace_back( i , ( 1. - sqrt( _l2 )/r ) * dt * 4 );
					}
					if( _leftButtonDown  )  selectLeft( selection );
					if( _rightButtonDown ) selectRight( selection );
					_overPosition = *p;
					_overVertex = vIdx;
					redisplay = true;
				}
				_depressedTime = _timer.elapsed();
			}
		}
		else if( animationCount )
		{
			if( animationCount!=static_cast< unsigned int >(-1) ) animationCount--;
			animate();
			redisplay = true;
		}
	}

	if( _overVertex!=static_cast< unsigned int >(-1) )
	{
		std::stringstream ss;
		Miscellany::StreamFloatPrecision sfp( ss , 3 );
		if( _mesh.values.size() ) ss << "Vertex[" << _overVertex << "]: " << _mesh.vertices[_overVertex] << " / " << _mesh.values[_overVertex];
		else                      ss << "Vertex[" << _overVertex << "]: " << _mesh.vertices[_overVertex];
		info[ _vertexIndex ] = ss.str();
	}

	info[ _animationIndex ] = std::string( "Animation: " ) + ( animationCount==static_cast< unsigned int >(-1) ? std::string( "ON" ) : std::string( "OFF" ) );

	if( redisplay ) glutPostRedisplay();
}

inline void DynamicMeshViewer::keyboardFunc( unsigned char key , int x , int y ){}

inline void DynamicMeshViewer::specialFunc( int key, int x, int y )
{
	switch( key )
	{
	case Visualization::KEY_UPARROW:    break;
	case Visualization::KEY_DOWNARROW:  break;
	case Visualization::KEY_LEFTARROW:  break;
	case Visualization::KEY_RIGHTARROW: break;
	case Visualization::KEY_PGUP:       break;
	case Visualization::KEY_PGDN:       break;
	}
}

inline void DynamicMeshViewer::OutputMouseInterfaceControls( std::ostream & out )
{
	out << "+----------------------------------------+" << std::endl;
	out << "| Mouse Interface Controls:              |" << std::endl;
	out << "|    [Left (drag)]:               rotate |" << std::endl;
	out << "|    [Right (drag)]:                zoom |" << std::endl;
	out << "|    [Left (drag)] + [CTRL]:         pan |" << std::endl;
	out << "|    [Left/Right (click)]: select vertex |" << std::endl;
	out << "+----------------------------------------+" << std::endl;
}