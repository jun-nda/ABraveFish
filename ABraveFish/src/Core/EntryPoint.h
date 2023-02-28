#pragma once

int main(int argc, char** argv)
{
	printf( "ABraveFish Soft Renderer" );
    auto app = ABraveFish::CreateApplication();
    app->Run();
	delete app;
}