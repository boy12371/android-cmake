$DEST      = "$ENV:KOKORO_ARTIFACTS_DIR\dest"
$OUT       = "$ENV:KOKORO_ARTIFACTS_DIR\out"
$CMAKE_SRC = "$ENV:KOKORO_ARTIFACTS_DIR\git\cmake_src"
$PYTHON    = "C:\python37\python"

pushd "$ENV:VS140COMNTOOLS..\..\VC"
cmd /c "vcvarsall.bat amd64 & set" |
foreach {
  if ($_ -match "=") {
    $v = $_.split("=")
    set-item -force -path "ENV:\$($v[0])"  -value "$($v[1])"
  }
}
popd

if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

$toolkit_path = (Resolve-Path "C:\Program Files*\Windows Kits\10\bin\*\x64\rc.exe")[-1] | Split-Path
$ENV:PATH += ";$toolkit_path"

# Remove cygwin from path so that cmake will not detect its compiler.
$ENV:PATH = ($ENV:PATH.Split(';') | Where-Object { $_ -notmatch 'cygwin' }) -join ';'

& $PYTHON --version
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

& $PYTHON "$PSScriptRoot\build.py", $CMAKE_SRC, $OUT, $DEST, $ENV:KOKORO_BUILD_ID,
  "--cmake=$ENV:KOKORO_ARTIFACTS_DIR\git\cmake\bin\cmake.exe",
  "--ninja=$ENV:KOKORO_ARTIFACTS_DIR\git\ninja\ninja.exe",
  "--android-cmake=$ENV:KOKORO_ARTIFACTS_DIR\git\android-cmake"
  "--clang-repo=$ENV:KOKORO_ARTIFACTS_DIR\git\clang"

exit $LASTEXITCODE
