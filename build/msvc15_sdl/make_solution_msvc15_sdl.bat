@echo off

if ["%~1"]==[""] (
  @echo invalid arguments, please select configuration
  goto end
)

set "CONFIGURATION=%1"

@echo Starting make solution SDL %CONFIGURATION% configuration...

@pushd ..
@call vcvarsall_msvc15.bat
@popd

@pushd ..
@call make_solution.bat "%CD%\..\CMake\Win32_SDL" solution_msvc15_sdl\%CONFIGURATION% "Visual Studio 15 2017" %CONFIGURATION% build_msvc15_sdl\%CONFIGURATION%
@popd

:end
@echo Done

@pause
