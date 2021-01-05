#include "GFkt.hpp"
#include "Matrix.hpp"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <fstream>
#include <iostream>
#include <iomanip>

GFkt::GFkt(std::shared_ptr<Domain> _grid) : u(_grid->ysize()+1, _grid->xsize()+1),
                                            grid(_grid) { } 
// TODO: fix copy assignment, make sure it works even though u, grid are private
GFkt::GFkt(const GFkt& gf) : u(gf.u), grid(gf.grid) { }

GFkt GFkt::detJinv() const {
  GFkt tmp = (dphix_dxi() * dphiy_deta()) - (dphiy_dxi() * dphix_deta());
  Matrix vals = tmp.get_values();
  for (int i = 0; i < vals.getRows(); ++i) {
    for (int j = 0; j < vals.getCols(); ++j) {
      if (vals[i][j] == 0.0) {
        vals[i][j] = 0.00001;
      }
      vals[i][j] = 1.0 / vals[i][j];
    }
  }
  tmp.set_values(vals); 
  return tmp;
}

GFkt& GFkt::operator=(const GFkt& gf) {
  if (this == &gf) {
    return *this;
  }
  u = gf.u;
  grid = gf.grid;
  return *this;
}

const GFkt& GFkt::operator+=(const GFkt& gf) {
  u += gf.u;
  return *this;
}

const GFkt GFkt::operator+(const GFkt& gf) const {
  // TODO: check if defined on the same grid
  // if (grid == gf.grid) { // defined on the same grid
    GFkt res(gf);
    res.u = u + gf.u;
    return res;
  // } else throw std::invalid_argument("Addition of grid functions require identical grids.");
}

const GFkt& GFkt::operator-=(const GFkt& gf) {
  u -= gf.u;
  return *this;
}

const GFkt GFkt::operator-(const GFkt& gf) const {
  // TODO: check if defined on the same grid
  // if (grid == gf.grid) { // defined on the same grid
    GFkt res(gf);
    res.u = u - gf.u;
    return res;
  // } else throw std::invalid_argument("Subtraction of grid functions require identical grids.");
}

const GFkt& GFkt::operator*=(const double k) {
  u *= k;
  return *this;
}

const GFkt GFkt::operator*(const GFkt& gf) const {
  // TODO: check if defined on the same grid
  
  // if (grid == gf.grid) { // defined on the same grid
    GFkt res(grid);
    for (int i = 0; i < grid->ysize() + 1; ++i) {
      for (int j = 0; j < grid->xsize() + 1; ++j) {
        res.u[i][j] = u[i][j] * gf.u[i][j];
      }
    }
    return res;
  // } else throw std::invalid_argument("Multiplication of grid functions require identical grids.");
}

const GFkt GFkt::operator*(const double k) const {
  GFkt res(*this);
  res *= k;
  return res;
}

const GFkt operator*(const double k, GFkt& gf) {
  return gf * k;
}

const GFkt& GFkt::operator/=(const double k) {
  u /= k;
  return *this;
}

const GFkt GFkt::operator/(const double k) const {
  GFkt res(*this);
  res /= k;
  return res; 
}

void GFkt::set_values(Matrix _u) {
  u = _u;
}

void GFkt::set_values(double (*f)(double x, double y)) {
  for (int i = 0; i < grid->ysize() + 1; ++i) {
    for (int j = 0; j < grid->xsize() + 1; ++j) {
      u[i][j] = f(grid->getPoint(i, j).x, grid->getPoint(i, j).y);
    }
  }
}

GFkt GFkt::dphix_dxi() const {
  // assume constant step size in xi
  GFkt tmp(grid);
  double h = 1.0 / grid->xsize();
  for (int i = 0; i < grid->ysize() + 1; ++i) {
    // j == 0
    tmp.u[i][0] = (1/h) * (grid->getPoint(i, 1).x - grid->getPoint(i, 0).x);
    for (int j = 1; j < grid->xsize(); ++j) {
      tmp.u[i][j] = (1/(2*h)) * (grid->getPoint(i, j+1).x - grid->getPoint(i, j-1).x);
    }
    // j == grid->xsize()
    tmp.u[i][grid->xsize()] = (1/h) * (grid->getPoint(i, grid->xsize()).x - grid->getPoint(i, grid->xsize() - 1).x);
  }
  return tmp;
}

GFkt GFkt::dphiy_dxi() const {
  // assume constant step size in xi
  GFkt tmp(grid);
  double h = 1.0 / grid->xsize();
  for (int i = 0; i < grid->ysize() + 1; ++i) {
    // j == 0
    tmp.u[i][0] = (1/h) * (grid->getPoint(i, 1).y - grid->getPoint(i, 0).y);
    for (int j = 1; j < grid->xsize(); ++j) {
      tmp.u[i][j] = (1/(2*h)) * (grid->getPoint(i, j+1).y - grid->getPoint(i, j-1).y);
    }
    // j == grid->xsize()
    tmp.u[i][grid->xsize()] = (1/h) * (grid->getPoint(i, grid->xsize()).y - grid->getPoint(i, grid->xsize() - 1).y);
  }
  return tmp;
}

GFkt GFkt::dphix_deta() const {
  // assume constant step size in eta
  GFkt tmp(grid);
  double h = 1.0 / grid->ysize();
  for (int j = 0; j < grid->xsize() + 1; ++j) {
    // i == 0
    tmp.u[0][j] = (1/h) * (grid->getPoint(1, j).x - grid->getPoint(0, j).x);
    for (int i = 1; i < grid->ysize(); ++i) {
      tmp.u[i][j] = (1/(2*h)) * (grid->getPoint(i+1, j).x - grid->getPoint(i-1, j).x);
    }
    // i == grid->ysize()
    tmp.u[grid->ysize()][j] = (1/h) * (grid->getPoint(grid->ysize(), j).x - grid->getPoint(grid->ysize() - 1, j).x);
  }
  return tmp;
}

GFkt GFkt::dphiy_deta() const {
  // assume constant step size in eta
  GFkt tmp(grid);
  double h = 1.0 / grid->ysize();
  for (int j = 0; j < grid->xsize() + 1; ++j) {
    // i == 0
    tmp.u[0][j] = (1/h) * (grid->getPoint(1, j).y - grid->getPoint(0, j).y);
    for (int i = 1; i < grid->ysize(); ++i) {
      tmp.u[i][j] = (1/(2*h)) * (grid->getPoint(i+1, j).y - grid->getPoint(i-1, j).y);
    }
    // i == grid->ysize()
    tmp.u[grid->ysize()][j] = (1/h) * (grid->getPoint(grid->ysize(), j).y - grid->getPoint(grid->ysize() - 1, j).y);
  }
  return tmp;
}


GFkt GFkt::du_dxi() const {
  // assume constant step size in xi
  GFkt tmp(grid);
  double h = 1.0 / grid->xsize();
  for (int i = 0; i < grid->ysize() + 1; ++i) {
    // j == 0
    tmp.u[i][0] = (1/h) * (u[i][1] - u[i][0]);
    for (int j = 1; j < grid->xsize(); ++j) {
      tmp.u[i][j] = (1/(2*h)) * (u[i][j+1] - u[i][j-1]);
    }
    //j == grid->xsize()
    tmp.u[i][grid->xsize()] = (1/h) * (u[i][grid->xsize()] - u[i][grid->xsize()-1]);
  }
  return tmp;
}

GFkt GFkt::du_deta() const {
  // assume constant step size in eta
  GFkt tmp(grid);
  double h = 1.0 / grid->ysize();
  for (int j = 0; j < grid->xsize() + 1; ++j) {
    // i == 0
    tmp.u[0][j] = (1/h) * (u[1][j] - u[0][j]);
    for (int i = 1; i < grid->ysize(); ++i) {
      tmp.u[i][j] = (1/(2*h)) * (u[i+1][j] - u[i-1][j]);
    }
    //i == grid->ysize()
    tmp.u[grid->ysize()][j] = (1/h) * (u[grid->ysize()][j] - u[grid->ysize() - 1][j]);
  }
  return tmp;
}

GFkt GFkt::du_dx() const {
  GFkt tmp = detJinv() * (du_dxi() * dphiy_deta() - du_deta() * dphiy_dxi());
  return tmp;
}

GFkt GFkt::du_dy() const {
  GFkt tmp = detJinv() * (du_deta() * dphix_dxi() - du_dxi() * dphix_deta());
  return tmp;
}

GFkt GFkt::Laplace() const {
  GFkt ddxx = (this->du_dx()).du_dx();
  GFkt ddyy = (this->du_dy()).du_dy();
  return ddxx + ddyy;  
}

Matrix GFkt::get_values() const {
  return this->u;
}

void GFkt::toFile(const char* filename) const {
  std::ofstream ofile;
  std::cout << "hello: " << filename << std::endl;
  ofile.open(filename);
  // if (!of) {
  //   std::cerr << "Error opening output file!";
  //   exit(1);
  // }
  ofile << grid->ysize() << ", ";
  ofile << grid->xsize() << "\n";
  const std::vector<double> xvals = grid->getX();
  const std::vector<double> yvals = grid->getY();
  const double* zvals = u.getArray();
  for (int i = 0; i < (grid->ysize() + 1)*(grid->xsize() + 1); ++i) {
    ofile << xvals[i] << ", ";
    ofile << yvals[i] << ", ";
    ofile << zvals[i] << "\n";
  }
}