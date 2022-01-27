$file ="$($args[0])"
$bytes  = [System.IO.File]::ReadAllBytes($file)
$bytes[5] = 1
[System.IO.File]::WriteAllBytes($file, $bytes)