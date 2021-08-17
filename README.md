# Cell Eye 

Cell Eye is a software for the analysis of time-lapse images of tumour spheroids as the cells migrate out of tumour bulk onto substrates of varying rigidity.
The program estimate the area of the tumour spheroids.

## Ubuntu 20.04 package

Download the latest package from `CellEye/packages/linux/pkg-debian/celleye_X.X.deb`. Install the package:

```
> sudo apt install ./celleye_X.X.deb
```

## Compile Cell Eye from source code.

###Getting Started

git clone repository
```
> git clone git@github.com:pablogalaviz/CellEye.git
> cd CellEye
```

### Installation

```
> mkdir build
> cd build 
> cmake .. 
> make 
> (sudo) make DESTDIR=/some/path/ install
```

## Usage
Run CellEye from command line.
```
> CellEye -h
Allowed options:
  --backup arg (=1)                     Create a backup of previous output
  -d [ --debug ]                        Shows debug messages in log
  -i [ --debug-image ]                  Shows image processing steps
  -o [ --output_path ] arg (=.CellEye)  Output directory name
  -p [ --preference_file ] arg (=.cell_eye.json)
                                        Preference file name
  -h [ --help ]                         Shows a help message
  -s [ --silent ]                       Shows only errors
```
## History

First release 17/08/2021

## Credits

Author: Pablo Galaviz              
Email:  pgalaviz@cmri.org.au


**Children’s Medical Research Institute, finding cures for childhood genetic diseases**

## License

CellEye is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
any later version.

CellEye is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with CellEye.  If not, see <http://www.gnu.org/licenses/>.
