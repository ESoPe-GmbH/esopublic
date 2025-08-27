# Expect the ip of the pcb as parameter
$ip = $args[0]

# Get eso filename from config.json
$jsonConfig = Get-Content -Path build/flasher_args.json | ConvertFrom-Json
$binObject = $jsonConfig.app
$projectname = $binObject.file
$binfile = $jsonConfig.app.file

Write-Debug "Update"
Write-Debug $binObject

# Update PCB using PUT
Invoke-WebRequest -InFile "build/$binfile" -Method 'POST' -Uri "http://$ip/ota"
#curl -v --data-binary "@publish/$projectname.eso" -X PUT "http://$ip/firmware"