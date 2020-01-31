#include <stdio.h>
#include <string.h>
#include <map>
#include <time.h> 
#include <math.h> 
#include <regex>

using namespace std; 


long get_index(char* st) {
    long rs=0; int i;
    for (i=0; st[i] != '"'; i++);
    for (i++; st[i] != '"'; i++)
        rs = 10 * rs + (st[i] - '0');
    return rs;
}
vector<long> reverse(vector<long> tmp) {
    vector<long> t2;
    for (int i = tmp.size() - 1; i >= 0; i--)
        t2.push_back(tmp[i]);
    return(t2);
}

void get_node_coord(const char* src_file, map<long, double>& vec_lat, map<long, double>& vec_lon) {    
    printf("Read node coordinate\n");
    char st[10000];
    long id;
    double lat, lon;
    // Read file for understand way/nodes
    long it = 0;
    FILE* f = fopen(src_file, "r");
    if (f == NULL) 
        printf("Error opening source file: \"%s\"\n", src_file); 
    fpos_t pos;
    while (feof(f) == 0) {
        fscanf(f, "%s", st);  
        if (strcmp("<node", st) == 0) {
            while (st[0] != 'i' || st[1] != 'd' || st[2] != '=')
                fscanf(f, "%s", st); 
            id = get_index(st);
            while (st[0] != 'l' || st[1] != 'a' || st[2] != 't' || st[3] != '=')
                fscanf(f, "%s", st); 
            lat = std::stod(st + 5);   
            while (st[0] != 'l' || st[1] != 'o' || st[2] != 'n' || st[3] != '=')
                fscanf(f, "%s", st); 
            lon = std::stod(st + 5);
            vec_lat.insert(pair<long, double>(id, lat));
            vec_lon.insert(pair<long, double>(id, lon));
        }
        it++;
        if (it == 10000000) {
            fgetpos(f, &pos);
            printf("get_node_coord %.2lf G read\n", 1.0 * (*(long*)(&pos)) / 1024 / 1024 / 1024);
            it = 0;
        }
    }
    fclose(f);            
    printf("Read node coordinate finish, found %lu\n", vec_lat.size());
}

double square(double* crd_lat, double* crd_lon, int n) {
    int i;
    double tmp;
    for (i = 0; i != n; i++) 
        tmp += crd_lon[i];
    tmp = tmp / n;
    // From gradus to km
    for (i = 0; i != n; i++) {
        crd_lon[i] = 2 * 3.141593 * 6378.1 * cos(crd_lat[i] / 180 * 3.141593) * (crd_lon[i] - tmp) / 360;
        crd_lat[i] = 2 * 3.141593 * 6378.1 * crd_lat[i] / 360;
    }
    double sq = 0;
    for (i = 1; i != n; i++) {
        sq += (crd_lat[i] - crd_lat[i - 1]) * (crd_lon[i] + crd_lon[i - 1]);
    }
    return .5 * fabs(sq) * 1e6;
}

void get_way(const char* src_file, map<long, double> vec_lat, map<long, double> vec_lon) {
    
  
    printf("Read way coordinate\n");
    char st[10000];
    long id;
    double lat, lon;
    
    int node_cnt;
    long node_way[100000];
    bool fl_way, build, tag_fl;
    float build_levels;
    long ref;
    int j, nbuild = 0, nbuild_lev = 0;
    
    
    map<string, double> build_stat;
    double tot_sq, sq;
    
    double crd_lat[100000];
    double crd_lon[100000];
    
    char buf[1000];
    string by_builing_name(src_file);
    by_builing_name.append(".by_bulding.csv");
    FILE* fout_lev = fopen(by_builing_name.data(), "w");
    fprintf(fout_lev, "id,lat,lon,levels,square\n");

    fpos_t pos;
    long it = 0;

    // Read file for understand way/nodes
    FILE* f = fopen(src_file, "r");
    while (feof(f) == 0) {
        fscanf(f, "%s", st);  
        if (strcmp("<way", st) == 0) {
            while (st[0] != 'i' && st[1] != 'd' && st[2] != '=')
                fscanf(f, "%s", st); 
            id = get_index(st);
            fl_way = true;
            node_cnt = 0;
            build = false;
            build_levels = -1;
            while (fl_way && strcmp("</way>", st) != 0) {
                // read node
                if (strcmp("<nd", st) == 0) {
                    fscanf(f, "%s", st); 
                    ref = get_index(st); 
                    if (vec_lat.count(ref) == 0)
                        fl_way = false;
                    node_way[node_cnt] = ref;
                    node_cnt++;
                }  else if (strcmp("<tag", st) == 0) {
                    tag_fl = true;
                    while (tag_fl) {
                        fscanf(f, "%s", st); 
                        if (strcmp("k=\"building\"", st) == 0)
                            build = true;
                        if (strcmp("k=\"building:levels\"", st) == 0) {
                            fscanf(f, "%s", st); 
                            sscanf(st + 3, "%f", &build_levels);  
                            build = true;
                        }
                        j = strlen(st);
                        tag_fl = j >= 2 && (st[j - 1] != '>' || st[j - 2] != '/');
                    }
                } 
                fscanf(f, "%s", st); 
                it++;
                if (it == 10000000) {
                    fgetpos(f, &pos);
                    printf("Read file for understand way/nodes %.2lf G read\n", 1.0 * (*(long*)(&pos)) / 1024 / 1024 / 1024);
                    it = 0;
                }
            }    
            
            it = 0;
            // Write results        
            if (fl_way && build) {
               //lat = 0; lon = 0;
               for (j = 0; j != node_cnt; j++) {
                    crd_lat[j] = vec_lat.at(node_way[j]);
                    crd_lon[j] = vec_lon.at(node_way[j]);
                    // lat += crd_lat[j]; 
                    // lon += crd_lon[j];
               }
               crd_lat[node_cnt] = crd_lat[0];
               crd_lon[node_cnt] = crd_lon[0];
               // lat = (lat + crd_lat[0]) / (node_cnt + 1);
               // lon = (lon + crd_lon[0]) / (node_cnt + 1);
               lat = crd_lat[0];
               lon = crd_lon[0];
               sq = square(crd_lat, crd_lon, node_cnt + 1);
               fprintf(fout_lev, "%d,%lf,%lf,%.1f,%lf\n", nbuild, lat, lon, build_levels, sq);
               
               lat = round(lat * 10) / 10;
               lon = round(lon * 10) / 10;

               // Build stat
               sprintf(buf, "%3.1lf,%3.1lf,%.1f", lat, lon, build_levels);
               if (build_stat.count(buf) == 0) {
                   build_stat.insert(pair<string, double> (buf, 0));
               }
               build_stat[buf] = build_stat[buf] + sq;
               nbuild++;
               tot_sq += sq;
               if (build_levels < 0) nbuild_lev++;
            }
        }
    }
    fclose(f); 
    fclose(fout_lev); 
    
    printf("Read all coordinate, found building %d (known level %d)\n", nbuild, nbuild_lev);
    
    printf("Write building statistics (%ld)\n", build_stat.size());
   
    string agg_stat_name(src_file);
    agg_stat_name.append(".agg_stat.csv");
    FILE* fout_stat = fopen(agg_stat_name.data(), "w");
    fprintf(fout_stat, "lat,lon,levels,square\n");    
    map<string, double>::iterator itr;
    for (itr = build_stat.begin(); itr != build_stat.end(); ++itr) { 
        fprintf(fout_stat, "%s,%lf\n", itr->first.data(), itr->second);
        tot_sq -= itr->second;
    } 
    fclose(fout_stat);
    printf("Check square: %lf\n", tot_sq);
    
}

void read_town(const char* src_file) {
 
    map<long, double> vec_lat;
    map<long, double> vec_lon;
    get_node_coord(src_file, vec_lat, vec_lon);
    get_way(src_file, vec_lat, vec_lon);
}
int main(int arc, char** argv) {
    if (arc != 2) {
        printf("You have define source file\n");
        return 1;
    }
    read_town(argv[1]);
    return 0;
}