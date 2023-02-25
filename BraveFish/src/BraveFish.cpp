#include "Application.h"
#include "EntryPoint.h"

ABraveFish::Application* ABraveFish::CreateApplication( )
{
	return new Application( );
}