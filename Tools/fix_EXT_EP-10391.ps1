# Fix for EXT_EP-10391
# CCS seems to break it's own makefiles during the convert from .d_raw to .d
# This script is a workaround until
# sir.ext.ti.com/.../EXT_EP-10391 is fixed
# D Thompson
# 9/6/2021

Get-ChildItem -Include *.d -Recurse | 
ForEach-Object {
    $filename = $_.FullName
    $content = Get-Content $_ 
    if ($content -match '\\ ') {
        Write-Output "Applying workaround EXT_EP-10391 to $filename"
        # if a backslash occurs mid-line - remove it.
        $content -Replace "\\ ", " " | Out-File $_ -Encoding utf8
      }
}