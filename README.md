# roofpv_potential

This package provides a simple tool for evaluating the potential of solar panel roofs based on OpenStreetMap data. This package processes the *.osm file and calculates the basic building statics needed to evaluate the solar potential of the roof. (for example, https://download.geofabrik.de/index.html can be used as a data source)

source: 
roof_buildings_statistics.cpp - source that gets the name of the source file and returns source.by_bulding.csv (by building - id,lat,lon,num_level,roof_square)
 and source.agg_stat.csv (at the aggregated level id,lat,lon,num_level,roof_square (lat & lon round by first digit)) and 
binary 
roof_buildings_statistics_linux_x64 
roof_buildings_statistics_win_x64.exe
