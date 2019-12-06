Scriptname TextureSet extends Form Hidden


; SKSE64 additions built 2019-11-21 05:06:53.443000 UTC

; Returns the number of texture paths
int Function GetNumTexturePaths() native

; Returns the path of the texture
string Function GetNthTexturePath(int n) native

; Sets the path of the texture
Function SetNthTexturePath(int n, string texturePath) native