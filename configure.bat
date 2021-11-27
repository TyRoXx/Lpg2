pushd vcpkg || exit /B 1
:: disable annoying message about metrics (we don't really care about metrics, but there seems to be no other way to make the message go away)
call bootstrap-vcpkg.bat -disableMetrics || exit /B 1
:: vcpkg install is missing a way to pass a triplet, so we have to set it for every package individually:
.\vcpkg.exe install --disable-metrics boost-system:x64-windows-static boost-test:x64-windows-static boost-outcome:x64-windows-static boost-filesystem:x64-windows-static ms-gsl:x64-windows-static benchmark:x64-windows-static || exit 1
popd || exit 1
cmake.exe -B ..\Lpg2_build -S . -G "Visual Studio 16 2019" -A x64 || exit 1
