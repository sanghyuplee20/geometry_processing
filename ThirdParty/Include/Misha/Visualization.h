/*
Copyright (c) 2018, Michael Kazhdan
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

#ifndef VISUALIZATION_INCLUDED
#define VISUALIZATION_INCLUDED

#include <algorithm>
#include <sstream>
#include <functional>
#include <algorithm>
#include <vector>
#include <GL/glew.h>
#include <GL/glut.h> 
#include <Misha/Exceptions.h>
#include <Misha/Miscellany.h>

namespace MishaK
{
	namespace Visualization
	{
		static const int KEY_UPARROW    = 101;
		static const int KEY_DOWNARROW	= 103;
		static const int KEY_LEFTARROW	= 100;
		static const int KEY_RIGHTARROW	= 102;
		static const int KEY_PGUP		= 104;
		static const int KEY_PGDN		= 105;
		static const int KEY_CTRL_C     =   3;
		static const int KEY_BACK_SPACE =   8;
		static const int KEY_ENTER      =  13;
		static const int KEY_ESC        =  27;

		struct Font
		{
			static const Font Fonts[];
			static const std::string FontNames[];

			enum Type
			{
				FONT_8_BY_13 ,
				FONT_9_BY_15 ,
				FONT_HELVETICA_10 ,
				FONT_HELVETICA_12 ,
				FONT_HELVETICA_18 ,
				FONT_TIMES_ROMAN_10 ,
				FONT_TIMES_ROMAN_24 ,
				FONT_COUNT
			};

			Font( void ) : _font(nullptr) , _fontHeight(0) {}

			unsigned int height( void ) const { return _fontHeight ;}
			void * operator()( void ) const { return _font; }
		protected:
			void * _font;
			unsigned int _fontHeight;

			Font( void * f , unsigned int fh ) :_font(f) , _fontHeight(fh) {}
		};

		const Font Font::Fonts[] =
		{
			Font( GLUT_BITMAP_8_BY_13 , 13 ) ,
			Font( GLUT_BITMAP_9_BY_15 , 15 ) ,
			Font( GLUT_BITMAP_HELVETICA_10 , 10 ) ,
			Font( GLUT_BITMAP_HELVETICA_12 , 12 ) ,
			Font( GLUT_BITMAP_HELVETICA_18 , 18 ) ,
			Font( GLUT_BITMAP_TIMES_ROMAN_10 , 10 ) ,
			Font( GLUT_BITMAP_TIMES_ROMAN_24 , 24 )
		};

		const std::string Font::FontNames[] =
		{
			"Bitmap 8x13" ,
			"Bitmap 9x15" ,
			"Helvetica 10" ,
			"Helvetica 12" ,
			"Helvetica 18" ,
			"Times-Roman 10" ,
			"Times-Roman 24"
		};

		using CallBackFunction = std::function< void ( std::string ) >;

		template< typename T >
		static CallBackFunction GetCallBackFunction( T * obj , void (T::*memberFunctionPointer)( std::string ) )
		{
			return [obj,memberFunctionPointer]( std::string str ){ return (obj->*memberFunctionPointer)( str ); };
		}

		struct KeyboardCallBack
		{
			struct Modifiers
			{
				bool alt , ctrl;
				Modifiers( void ) : alt(false) , ctrl(false){}
				Modifiers( bool alt , bool ctrl ) : alt(alt) , ctrl(ctrl) {};
				bool operator == ( const Modifiers &m ) const { return alt==m.alt && ctrl==m.ctrl; }
				bool operator != ( const Modifiers &m ) const { return alt!=m.alt || ctrl!=m.ctrl; }
			};

			char key;
			std::string prompt;
			std::string description;
			CallBackFunction callBackFunction;
			Modifiers modifiers;

			KeyboardCallBack( char key , Modifiers modifiers , std::string description ,                      CallBackFunction callBackFunction );
			KeyboardCallBack( char key , Modifiers modifiers , std::string description , std::string prompt , CallBackFunction callBackFunction );
			template< typename T >
			KeyboardCallBack( char key , Modifiers modifiers , std::string description ,                      T * obj , void (T::*memberFunctionPointer)( std::string ) );
			template< typename T >
			KeyboardCallBack( char key , Modifiers modifiers , std::string description , std::string prompt , T * obj , void (T::*memberFunctionPointer)( std::string ) );
		};

		template< typename DerivedViewableType >
		struct Viewable
		{
			struct Viewer
			{
				static DerivedViewableType * viewable;
				static void Run( DerivedViewableType * viewable , int argc , char * argv[] , std::string windowName="" );
				static void Idle             ( void );
				static void KeyboardFunc     ( unsigned char key , int x , int y );
				static void KeyboardUpFunc   ( unsigned char key , int x , int y );
				static void SpecialFunc      ( int key, int x, int y );
				static void SpecialUpFunc    ( int key, int x, int y );
				static void Display          ( void );
				static void Reshape          ( int w , int h );
				static void MouseFunc        ( int button , int state , int x , int y );
				static void MouseWheelFunc   ( int button , int state , int x , int y );
				static void MotionFunc       ( int x , int y );
				static void PassiveMotionFunc( int x , int y );
			};

			unsigned int screenWidth , screenHeight;
			Font font , promptFont;
			bool showHelp , showInfo , showFPS;
			CallBackFunction promptCallBack;
			std::string promptString;
			unsigned int originalPromptLength;
			std::function< void ( void ) > quitFunction;

			std::vector< KeyboardCallBack > callBacks;
			std::vector< std::string > info;
			Viewable( void );
			void setFont( unsigned int idx );
			virtual void display( void ) {}
			virtual void idle( void ) {}
			virtual void keyboardFunc( unsigned char key , int x , int y ) {}
			virtual void keyboardUpFunc( unsigned char key , int x , int y ) {}
			virtual void specialFunc( int key, int x, int y ) {}
			virtual void specialUpFunc( int key, int x, int y ) {}
			virtual void mouseFunc( int button , int state , int x , int y ) {}
			virtual void mouseWheelFunc( int button , int state , int x , int y ) {}
			virtual void motionFunc( int x , int y ) {}
			virtual void passiveMotionFunc( int x , int y ) {}
			virtual void reshape( int w , int h )
			{
				screenWidth = w , screenHeight = h;
				glViewport( 0 , 0 , screenWidth , screenHeight );
			}

			void Idle             ( void );
			void KeyboardFunc     ( unsigned char key , int x , int y );
			void KeyboardUpFunc   ( unsigned char key , int x , int y );
			void SpecialFunc      ( int key, int x, int y );
			void SpecialUpFunc    ( int key, int x, int y );
			void Display          ( void );
			void Reshape          ( int w , int h );
			void MouseFunc        ( int button , int state , int x , int y );
			void MouseWheelFunc   ( int button , int state , int x , int y );
			void MotionFunc       ( int x , int y );
			void PassiveMotionFunc( int x , int y );

			void       exitCallBack( std::string ){ exit( 0 ); }
			void       quitCallBack( std::string ){ quitFunction() , exit( 0 ); }
			void  toggleFPSCallBack( std::string ){ showFPS  = !showFPS ; }
			void toggleHelpCallBack( std::string ){ showHelp = !showHelp; }
			void toggleInfoCallBack( std::string ){ showInfo = !showInfo; }

			static void WriteLeftString( int x , int y , const Font & font , std::string );
			static unsigned int StringWidth( const Font & font , std::string );
			void writeLeftString( int x , int y , std::string ) const;
			void writeRightString( int x , int y , std::string ) const;
			void writeCenterString( int x , int y , std::string ) const;
			unsigned int stringWidth( std::string ) const;

			void setPromptCallBack( std::string prompt , CallBackFunction callBackFunction );

			void addCallBack( char key , typename KeyboardCallBack::Modifiers modifiers , std::string description ,                      CallBackFunction callBackFunction );
			void addCallBack( char key , typename KeyboardCallBack::Modifiers modifiers , std::string description , std::string prompt , CallBackFunction callBackFunction );
			void addCallBack( char key ,                                                  std::string description ,                      CallBackFunction callBackFunction );
			void addCallBack( char key ,                                                  std::string description , std::string prompt , CallBackFunction callBackFunction );
			template< typename V >
			void addCallBack( char key , typename KeyboardCallBack::Modifiers modifiers , std::string description ,                      void (V::*memberFunctionPointer)( std::string ) );
			template< typename V >
			void addCallBack( char key , typename KeyboardCallBack::Modifiers modifiers , std::string description , std::string prompt , void (V::*memberFunctionPointer)( std::string ) );
			template< typename V >
			void addCallBack( char key ,                                                  std::string description ,                      void (V::*memberFunctionPointer)( std::string ) );
			template< typename V >
			void addCallBack( char key ,                                                  std::string description , std::string prompt , void (V::*memberFunctionPointer)( std::string ) );

		protected:
			const double _MIN_FPS_TIME = 0.5;
			Miscellany::Timer _timer;
			unsigned int _lastFPSCount;
			double _fps;
			int _currentFrame , _totalFrames;
		};

		//////////////////////
		// KeyboardCallBack //
		//////////////////////
		KeyboardCallBack::KeyboardCallBack( char key , Modifiers modifiers , std::string description , CallBackFunction callBackFunction )
			: modifiers(modifiers) , key(key) , description(description) , callBackFunction(callBackFunction)
		{}

		KeyboardCallBack::KeyboardCallBack( char key , Modifiers modifiers , std::string description , std::string prompt , CallBackFunction callBackFunction )
			: modifiers(modifiers) , key(key) , description(description) , prompt(prompt) , callBackFunction(callBackFunction)
		{}

		template< typename T >
		KeyboardCallBack::KeyboardCallBack( char key , Modifiers modifiers , std::string description , T * obj , void (T::*memberFunctionPointer)( std::string ) )
			: modifiers(modifiers) , key(key) , description(description) , callBackFunction( GetCallBackFunction(obj,memberFunctionPointer) )
		{}

		template< typename T >
		KeyboardCallBack::KeyboardCallBack( char key , Modifiers modifiers , std::string description , std::string prompt , T * obj , void (T::*memberFunctionPointer)( std::string ) )
			: modifiers(modifiers) , key(key) , description(description) , prompt(prompt) , callBackFunction( GetCallBackFunction(obj,memberFunctionPointer) )
		{}

		//////////////////////
		// Viewable::Viewer //
		//////////////////////

		template< typename DerivedViewableType > DerivedViewableType* Viewable< DerivedViewableType >::Viewer::viewable = nullptr;

		template< typename DerivedViewableType >
		void Viewable< DerivedViewableType >::Viewer::Run( DerivedViewableType * v , int argc , char * argv[] , std::string windowName )
		{
			viewable = v;
			glutInitDisplayMode( GLUT_DEPTH | GLUT_RGBA | GLUT_DOUBLE );
			glutInitWindowSize( viewable->screenWidth , viewable->screenHeight );
			glutInit( &argc , argv );
			glutCreateWindow( windowName.c_str() );

			if( glewInit()!=GLEW_OK ) MK_ERROR_OUT( "glewInit failed" );

			glutIdleFunc         ( Idle );
			glutDisplayFunc      ( Display );
			glutReshapeFunc      ( Reshape );
			glutMouseFunc        ( MouseFunc );
			glutMotionFunc       ( MotionFunc );
			glutPassiveMotionFunc( PassiveMotionFunc );
			glutKeyboardFunc     ( KeyboardFunc );
			glutKeyboardUpFunc   ( KeyboardUpFunc );
			glutSpecialFunc      ( SpecialFunc );
			glutSpecialUpFunc    ( SpecialUpFunc );

			glutMainLoop();
		}

		template< typename DerivedViewableType > void Viewable< DerivedViewableType >::Viewer::Idle( void ){ viewable->Idle(); }
		template< typename DerivedViewableType > void Viewable< DerivedViewableType >::Viewer::KeyboardFunc( unsigned char key , int x , int y ){ viewable->KeyboardFunc( key , x , y ); }
		template< typename DerivedViewableType > void Viewable< DerivedViewableType >::Viewer::KeyboardUpFunc( unsigned char key , int x , int y ){ viewable->KeyboardUpFunc( key , x , y ); }
		template< typename DerivedViewableType > void Viewable< DerivedViewableType >::Viewer::SpecialFunc( int key , int x , int y ){ viewable->SpecialFunc( key , x ,  y ); }
		template< typename DerivedViewableType > void Viewable< DerivedViewableType >::Viewer::SpecialUpFunc( int key , int x , int y ){ viewable->SpecialUpFunc( key , x ,  y ); }
		template< typename DerivedViewableType > void Viewable< DerivedViewableType >::Viewer::Display( void ){ viewable->Display(); }
		template< typename DerivedViewableType > void Viewable< DerivedViewableType >::Viewer::Reshape( int w , int h ){ viewable->Reshape( w , h ); }
		template< typename DerivedViewableType > void Viewable< DerivedViewableType >::Viewer::MouseFunc( int button , int state , int x , int y ){ viewable->MouseFunc( button , state , x , y ); }
		template< typename DerivedViewableType > void Viewable< DerivedViewableType >::Viewer::MotionFunc( int x , int y ){ viewable->MotionFunc( x , y ); }
		template< typename DerivedViewableType > void Viewable< DerivedViewableType >::Viewer::PassiveMotionFunc( int x , int y ){ viewable->PassiveMotionFunc( x , y ); }

		//////////////
		// Viewable //
		//////////////
		template< typename DerivedViewableType > void Viewable< DerivedViewableType >::Reshape( int w , int h ){ reshape( w , h ); }
		template< typename DerivedViewableType > void Viewable< DerivedViewableType >::MouseFunc( int button , int state , int x , int y ){ mouseFunc( button , state , x , y ); }
		template< typename DerivedViewableType > void Viewable< DerivedViewableType >::MouseWheelFunc( int button , int state , int x , int y ){ mouseWheelFunc( button , state , x , y ); }
		template< typename DerivedViewableType > void Viewable< DerivedViewableType >::MotionFunc( int x , int y ){ motionFunc( x , y );}
		template< typename DerivedViewableType > void Viewable< DerivedViewableType >::PassiveMotionFunc( int x , int y ){ passiveMotionFunc( x , y );}
		template< typename DerivedViewableType > 
		void Viewable< DerivedViewableType >::Idle( void ){ idle(); }

		template< typename DerivedViewableType >
		void Viewable< DerivedViewableType >::setPromptCallBack( std::string prompt , CallBackFunction callBackFunction )
		{
			promptString = prompt + std::string( ": " );
			originalPromptLength = static_cast< unsigned int >( promptString.size() );
			promptCallBack = callBackFunction;
		}

		template< typename DerivedViewableType >
		void Viewable< DerivedViewableType >::KeyboardFunc( unsigned char key , int x , int y )
		{
			if( promptCallBack )
			{
				size_t len = promptString.size();
				if( key==KEY_BACK_SPACE )
				{
					if( len>originalPromptLength ) promptString.pop_back();
				}
				else if( key==KEY_ENTER )
				{
					promptCallBack( promptString.substr( originalPromptLength ) );
					promptString.clear();
					originalPromptLength = 0;
					promptCallBack = nullptr;
				}
				else if( key==KEY_CTRL_C )
				{
					promptString.clear();
					originalPromptLength = 0;
					promptCallBack = nullptr;
				}
				else if( key>=32 && key<=126 ) // ' ' to '~'
				{
					promptString.push_back( key );
				}
				glutPostRedisplay();
				return;
			}
			switch( key )
			{
			case KEY_CTRL_C:
				exit( 0 );
				break;
			default:
			{
				int m = glutGetModifiers();
				typename KeyboardCallBack::Modifiers modifiers( m & GLUT_ACTIVE_ALT , m & GLUT_ACTIVE_CTRL );
				for( unsigned int i=0 ; i<callBacks.size() ; i++ ) if( callBacks[i].key==key && callBacks[i].modifiers==modifiers )
				{
					if( callBacks[i].prompt.size() ) setPromptCallBack( callBacks[i].prompt , callBacks[i].callBackFunction );
					else callBacks[i].callBackFunction( std::string() );
					break;
				}
			}
			}
			keyboardFunc( key , x , y );
			glutPostRedisplay();
		}

		template< typename DerivedViewableType > void Viewable< DerivedViewableType >::KeyboardUpFunc( unsigned char key , int x , int y ){ keyboardUpFunc( key , x , y ); }

		template< typename DerivedViewableType > void Viewable< DerivedViewableType >::SpecialFunc( int key , int x , int y ){ specialFunc( key , x , y ); }

		template< typename DerivedViewableType > void Viewable< DerivedViewableType >::SpecialUpFunc( int key , int x , int y ){ specialUpFunc( key , x , y ); }

		template< typename DerivedViewableType >
		void Viewable< DerivedViewableType >::Display( void )
		{
			glClearColor( 1 , 1 , 1 , 1 );
			glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

			display();

			_lastFPSCount++;

			double dt = _timer.elapsed();
			if( dt > _MIN_FPS_TIME )
			{
				_fps = (double)_lastFPSCount / dt;
				_lastFPSCount = 0;
				_timer.reset();
			}

			if( showFPS )
			{
				std::stringstream ss;
				Miscellany::StreamFloatPrecision sfp( ss , 2 );
				ss << screenWidth << " x " << screenHeight << " @ " << _fps;
				writeRightString( 5 , screenHeight - font.height() - 5 , ss.str() );
			}

			GLboolean writeMask;
			glGetBooleanv( GL_DEPTH_WRITEMASK , &writeMask );
			glDepthMask( GL_FALSE );

			glDisable( GL_LIGHTING );
			unsigned int offset = font.height()/2;
			if( showHelp )
			{
				auto CallBackString = [&]( int i )
					{
						std::stringstream stream;
						stream << "\'" << callBacks[i].key << "\'";
						if( callBacks[i].modifiers.ctrl ) stream << "+[CTRL]";
						if( callBacks[i].modifiers.alt ) stream << "+[ALT]";
						stream << ": " << callBacks[i].description;
						return stream.str();
					};
				{
					GLint vp[4];
					glGetIntegerv( GL_VIEWPORT , vp );

					glMatrixMode( GL_PROJECTION );
					glPushMatrix();
					glLoadIdentity();
					glOrtho( vp[0] , vp[2] , vp[1] , vp[3] , 0 , 1 );

					glMatrixMode( GL_MODELVIEW );
					glPushMatrix();
					glLoadIdentity();

					int x=0 , y = offset;
					for( unsigned int i=0 ; i<callBacks.size() ; i++ ) if( callBacks[i].description.size() ) x = std::max< int >( x , stringWidth( CallBackString(i).c_str() ) ) , y += font.height() + offset;

					GLint srcAlpha , dstAlpha;
					glGetIntegerv( GL_BLEND_SRC_ALPHA , &srcAlpha );
					glGetIntegerv( GL_BLEND_DST_ALPHA , &dstAlpha );

					glEnable( GL_BLEND );
					glBlendFunc( GL_SRC_ALPHA , GL_ONE_MINUS_SRC_ALPHA );
					glBegin( GL_QUADS );
					glColor4f( 1.f , 1.f , 1.f , 0.5f );
					glVertex2i( screenWidth-5 , 0 ) , glVertex2i( screenWidth-(x+15) , 0 ) , glVertex2i( screenWidth-(x+15) , y ) , glVertex2i( screenWidth-5 , y );
					glEnd();
					glBlendFunc( srcAlpha , dstAlpha );
					glDisable( GL_BLEND );
					glDisable( GL_DEPTH_TEST );
					glLineWidth( 2.f );
					glBegin( GL_LINE_LOOP );
					glColor4f( 0.f , 0.f , 0.f , 1.f );
					glVertex2i( screenWidth-5 , 0 ) , glVertex2i( screenWidth-(x+15) , 0 ) , glVertex2i( screenWidth-(x+15) , y ) , glVertex2i( screenWidth-5 , y );
					glEnd();

					glMatrixMode( GL_PROJECTION );
					glPopMatrix();

					glMatrixMode( GL_MODELVIEW );
					glPopMatrix();
				}

				{
					int y = offset , width = 0;

					for( unsigned int i=0 ; i<callBacks.size() ; i++ ) if( callBacks[i].description.size() )
						width = std::max< int >( width , stringWidth( CallBackString(i).c_str() ) );
					for( unsigned int i=0 ; i<callBacks.size() ; i++ ) if( callBacks[i].description.size() )
						writeLeftString( screenWidth - 10 - width , y , CallBackString(i).c_str() ) , y += font.height() + offset;
				}
			}
			if( showInfo && info.size() )
			{
				{
					GLint vp[4];
					glGetIntegerv( GL_VIEWPORT , vp );

					glMatrixMode( GL_MODELVIEW );
					glPushMatrix();
					glLoadIdentity();
					glMatrixMode( GL_PROJECTION );
					glPushMatrix();
					glLoadIdentity();
					glOrtho( vp[0] , vp[2] , vp[1] , vp[3] , 0 , 1 );
					int x=0 , y = offset;
					for( unsigned int i=0 ; i<info.size() ; i++ ) if( info[i].size() ) x = std::max< int >( x , glutBitmapLength( font() , reinterpret_cast< const unsigned char * >( info[i].c_str() ) ) ) , y += font.height() + offset;
					glEnable( GL_BLEND );
					GLint srcAlpha , dstAlpha;
					glGetIntegerv( GL_BLEND_SRC_ALPHA , &srcAlpha );
					glGetIntegerv( GL_BLEND_DST_ALPHA , &dstAlpha );
					glBlendFunc( GL_SRC_ALPHA , GL_ONE_MINUS_SRC_ALPHA );
					glBegin( GL_QUADS );
					glColor4f( 1.f , 1.f , 1.f , 0.5f );
					glVertex2i( 5 , 0 ) , glVertex2i( x+15 , 0 ) , glVertex2i( x+15 , y ) , glVertex2i( 5 , y );
					glEnd();
					glBlendFunc( srcAlpha , dstAlpha );
					glDisable( GL_BLEND );
					glDisable( GL_DEPTH_TEST );
					glLineWidth( 2.f );
					glBegin( GL_LINE_LOOP );
					glColor4f( 0.f , 0.f , 0.f , 1.f );
					glVertex2i( 5 , 0 ) , glVertex2i( x+15 , 0 ) , glVertex2i( x+15 , y ) , glVertex2i( 5 , y );
					glEnd();

					glMatrixMode( GL_PROJECTION );
					glPopMatrix();

					glMatrixMode( GL_MODELVIEW );
					glPopMatrix();
				}
				{
					int y = offset;
					for( unsigned int i=0 ; i<info.size() ; i++ ) if( info[i].size() ) writeLeftString( 10 , y , info[i] ) , y += font.height() + offset;
				}
			}
			if( promptString.size() )
			{
				Font _font = font;
				font = promptFont;

				int sw = StringWidth ( font , promptString );
				glColor4f( 1.f , 1.f , 1.f , 0.5 );
				glEnable( GL_BLEND );
				GLint srcAlpha , dstAlpha;
				glGetIntegerv( GL_BLEND_SRC_ALPHA , &srcAlpha );
				glGetIntegerv( GL_BLEND_DST_ALPHA , &dstAlpha );
				glBlendFunc( GL_SRC_ALPHA , GL_ONE_MINUS_SRC_ALPHA );
				glBegin( GL_QUADS );
				{
					glVertex2i(     0 , screenHeight );
					glVertex2i( sw+20 , screenHeight );
					glVertex2i( sw+20 , screenHeight-font.height()*2 );
					glVertex2i(     0 , screenHeight-font.height()*2 );
				}
				glEnd();
				glBlendFunc( srcAlpha , dstAlpha );
				glDisable( GL_BLEND );
				glColor4f( 0.f , 0.f , 0.f , 1.f );
				glLineWidth( 2.f );
				glBegin( GL_LINE_LOOP );
				{
					glVertex2i(     0 , screenHeight );
					glVertex2i( sw+20 , screenHeight );
					glVertex2i( sw+20 , screenHeight-font.height()*2 );
					glVertex2i(     0 , screenHeight-font.height()*2 );
				}
				glEnd();
				writeLeftString( 10 , screenHeight-font.height()-font.height()/2 , promptString.c_str() );
				font = _font;
			}
			if( writeMask ) glDepthMask( GL_TRUE );
			glutSwapBuffers();
		}

		template< typename DerivedViewableType >
		void Viewable< DerivedViewableType >::WriteLeftString( int x , int y , const Font & font , std::string str )
		{
			GLint vp[4];
			glGetIntegerv( GL_VIEWPORT , vp );

			glMatrixMode( GL_PROJECTION );
			glPushMatrix();
			glLoadIdentity();
			glOrtho( vp[0] , vp[2] , vp[1] , vp[3] , 0 , 1 );

			glMatrixMode( GL_MODELVIEW );
			glPushMatrix();
			glLoadIdentity();

			GLint matrixMode;
			glGetIntegerv( GL_MATRIX_MODE , &matrixMode );
			int depth = glIsEnabled( GL_DEPTH_TEST );
			int lighting = glIsEnabled( GL_LIGHTING );
			glDisable( GL_DEPTH_TEST );
			glDisable( GL_LIGHTING );
			glColor4f( 0 , 0 , 0 , 1 );
			glRasterPos2i( x , y );
			for( unsigned int i=0 ; i<str.size() ; i++ ) glutBitmapCharacter( font() , str[i] );
			if( depth ) glEnable( GL_DEPTH_TEST );
			if( lighting ) glEnable( GL_LIGHTING );

			glMatrixMode( GL_PROJECTION );
			glPopMatrix();

			glMatrixMode( GL_MODELVIEW );
			glPopMatrix();

			glMatrixMode( matrixMode );
		}

		template< typename DerivedViewableType >
		unsigned int Viewable< DerivedViewableType >::StringWidth( const Font & font , std::string str )
		{
			return glutBitmapLength( font() , reinterpret_cast< const unsigned char * >( str.c_str() ) );
		}

		template< typename DerivedViewableType >
		unsigned int Viewable< DerivedViewableType >::stringWidth( std::string str ) const
		{
			return glutBitmapLength( font() , reinterpret_cast< const unsigned char * >( str.c_str() ) );
		}

		template< typename DerivedViewableType >
		void Viewable< DerivedViewableType >::writeLeftString( int x , int y , std::string str ) const
		{
			WriteLeftString( x , y , font , str );
		}

		template< typename DerivedViewableType >
		void Viewable< DerivedViewableType >::writeRightString( int x , int y , std::string str ) const
		{
			WriteLeftString( screenWidth-x-stringWidth( str ) , y , font  ,str );
		}

		template< typename DerivedViewableType >
		void Viewable< DerivedViewableType >::writeCenterString( int x , int y , std::string str ) const
		{
			WriteLeftString( x-stringWidth( str )/2 , y , font , str );
		}

		template< typename DerivedViewableType >
		void Viewable< DerivedViewableType >::addCallBack( char key , typename KeyboardCallBack::Modifiers modifiers , std::string description , CallBackFunction callBackFunction )
		{
			callBacks.push_back( KeyboardCallBack( key , modifiers , description , callBackFunction ) );
		}

		template< typename DerivedViewableType >
		void Viewable< DerivedViewableType >::addCallBack( char key , typename KeyboardCallBack::Modifiers modifiers , std::string description , std::string prompt , CallBackFunction callBackFunction )
		{
			callBacks.push_back( KeyboardCallBack( key , modifiers , description , prompt , callBackFunction ) );
		}

		template< typename DerivedViewableType >
		void Viewable< DerivedViewableType >::addCallBack( char key , std::string description , CallBackFunction callBackFunction )
		{
			callBacks.push_back( KeyboardCallBack( key , KeyboardCallBack::Modifiers() , description , callBackFunction ) );
		}

		template< typename DerivedViewableType >
		void Viewable< DerivedViewableType >::addCallBack( char key , std::string description , std::string prompt , CallBackFunction callBackFunction )
		{
			callBacks.push_back( KeyboardCallBack( key , KeyboardCallBack::Modifiers() , description , prompt , callBackFunction ) );
		}

		template< typename DerivedViewableType >
		template< typename V >
		void Viewable< DerivedViewableType >::addCallBack( char key , typename KeyboardCallBack::Modifiers modifiers , std::string description , void (V::*memberFunctionPointer)( std::string ) )
		{
			callBacks.push_back( KeyboardCallBack( key , modifiers , description , reinterpret_cast< V * >( this ) , memberFunctionPointer ) );
		}

		template< typename DerivedViewableType >
		template< typename V >
		void Viewable< DerivedViewableType >::addCallBack( char key , typename KeyboardCallBack::Modifiers modifiers , std::string description , std::string prompt , void (V::*memberFunctionPointer)( std::string ) )
		{
			callBacks.push_back( KeyboardCallBack( key , modifiers , description , prompt , reinterpret_cast< V * >( this ) , memberFunctionPointer ) );
		}

		template< typename DerivedViewableType >
		template< typename V >
		void Viewable< DerivedViewableType >::addCallBack( char key , std::string description , void (V::*memberFunctionPointer)( std::string ) )
		{
			callBacks.push_back( KeyboardCallBack( key , KeyboardCallBack::Modifiers() , description , reinterpret_cast< V * >( this ) , memberFunctionPointer ) );
		}

		template< typename DerivedViewableType >
		template< typename V >
		void Viewable< DerivedViewableType >::addCallBack( char key , std::string description , std::string prompt , void (V::*memberFunctionPointer)( std::string ) )
		{
			callBacks.push_back( KeyboardCallBack( key , KeyboardCallBack::Modifiers() , description , prompt , reinterpret_cast< V * >( this ) , memberFunctionPointer ) );
		}

		template< typename DerivedViewableType >
		Viewable< DerivedViewableType >::Viewable( void )
		{
			quitFunction = []( void ){};
			addCallBack( KEY_ESC    , ""            , &Viewable::quitCallBack );
			addCallBack( KEY_CTRL_C , ""            , &Viewable::exitCallBack );
			addCallBack( 'F'        , "toggle fps"  , &Viewable::toggleFPSCallBack );
			addCallBack( 'H'        , "toggle help" , &Viewable::toggleHelpCallBack );
			addCallBack( 'I'        , "toggle info" , &Viewable::toggleInfoCallBack );
			showHelp = showInfo = showFPS = true;
			screenWidth = screenHeight = 512;
			font = Font::Fonts[ Font::Type::FONT_HELVETICA_12 ];
			promptFont = Font::Fonts[ Font::Type::FONT_TIMES_ROMAN_24 ];
			promptCallBack = nullptr;
			originalPromptLength = 0;

			_timer.reset();
			_lastFPSCount = 0;
			_fps = 0;
		}

		template< typename DerivedViewableType >
		void Viewable< DerivedViewableType >::setFont( unsigned int idx )
		{
			if( idx>=Font::Type::FONT_COUNT ) MK_WARN( "Font index out of bounds: " , idx , " >= " , Font::Type::FONT_COUNT );
			else font = Font::Fonts[idx];
		}
	}
}
#endif // VISUALIZATION_INCLUDED