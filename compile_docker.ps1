if (!(Get-Command docker -ErrorAction SilentlyContinue)) {
    Write-Error "Docker not installed.";
    Write-Output "Install Docker at https://www.docker.com/";
    exit 1;
}

docker -v
if (!$?) {
    Write-Error "Docker is not running. Start Docker Desktop and try again.";
    exit 1;
}

# Rebuild the TWiLightMenu image if it doesn't exist or the Dockerfile has changed since it was built.
$dockerfileHash = (Get-FileHash -Algorithm SHA256 "$PSScriptRoot\Dockerfile").Hash.ToLower()
$imageHash = $null
$labelsJson = docker image inspect twilightmenu --format '{{json .Config.Labels}}' 2>$null
if ($LASTEXITCODE -eq 0 -and $labelsJson) {
    $imageHash = ($labelsJson | ConvertFrom-Json).'dockerfile-hash'
}

if ($imageHash -ne $dockerfileHash) {
    docker build -t twilightmenu --label twilightmenu --label "dockerfile-hash=$dockerfileHash" "$PSScriptRoot"
    if (!$?) {
        Write-Error "Failed to build the Docker image.";
        exit 1;
    }
}

# Only request a TTY when run interactively; "-t -i" fails when input is redirected.
$ttyArgs = if ([Console]::IsInputRedirected) { @() } else { @("-t", "-i") }

docker run --rm @ttyArgs -v "${PSScriptRoot}:/data" twilightmenu make @args
exit $LASTEXITCODE
