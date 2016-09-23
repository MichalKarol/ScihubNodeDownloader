# ScihubNodeDownloader
Tool to download data nodes from Sentinel-1/2 products. Save time and bandwith by requesting only data you need.

## Sentinel-2 Single tile
1. Open wanted product
2. Create main product directory named like the value of that field 'filename' in metadata
3. Create directory "GRANULE" inside it
4. Download product xml (It is in first node and is named $PRODUCTNAME.xml) to main product directory
5. Download tile node into 'GRANULE' directory
6. Open it with SNAP

or 

1. Download bands you need.
2. Open bands in SNAP
3. Select one band as master and add other bands to it (via adding virtual bands)

