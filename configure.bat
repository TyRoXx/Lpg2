pushd vcpkg || exit /B 1
:: disable annoying message about metrics (we don't really care about metrics, but there seems to be no other way to make the message go away)
call bootstrap-vcpkg.bat -disableMetrics || exit /B 1
:: vcpkg install is missing a way to pass a triplet, so we have to set it for every package individually:
.\vcpkg.exe install --disable-metrics --overlay-triplets=.. --triplet=vcpkg-msvc boost-system boost-outcome ms-gsl benchmark catch2 || exit /B 1
.\vcpkg.exe upgrade --no-dry-run --overlay-triplets=.. || exit /B 1
popd || exit /B 1
.\vcpkg\downloads\tools\cmake-3.27.1-windows\cmake-3.27.1-windows-i386\bin\cmake.exe -B ..\Lpg2_build2022 -S . -G "Visual Studio 17 2022" -A x64 || exit 1
