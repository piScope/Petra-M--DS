#
#   mumps_driver
#      this module provide
#         a generic interface to solve a linear system usig mumps
#
#       it does 
#         1) select a solver depending on data type
#         2) perfrom index type consistency check
#         3) provide a functional interface to solver
#
#    a user can do either,,,
#
#    1) a functional interface
#      from petram.solver.mumps import solve
#      solution = solve(matrix, rhs)
#
#    2) an object interface
#      from petram.solver.mumps import MUMPS
#
#      s = MUMPS(dtype = 'complex128')
#      s.set_data(row, col, mat, rhs)
#      sol = s.run()
#      s.finish()

from mumps_solve import dmumps, smumps, cmump, zmumps, i_array, z_array, c_array, s_array, d_array

class MUMPS():
    def __init__(dtype = 'complex128'):
        if dtype == 'complex128':
            self.solver = zmumps()
        elif dtype == 'complex64':
            self.solver = cmumps()
        elif dtype == 'float64':
            self.solver = dmumps()            
        elif dtype == 'float32':
            self.solver = smumps()            
        else:
            raise ValueError('Unsupported dtype')
        self.solver = None
        self.dtype = dtype
        
    def __getattr__(self, attr):
      # map attribute query to self.solver
       if hasattr(self.solver, attr):
           return getattr(self.solver, attr)
       else:
           raise AttributeError('Solver does not have attribute '+str(attr))
    
    def set_data(self, row, col, data, rhs, local = False):
        if data.dtype != self.dtype:
           raise ValueError('solver dtype does not match with data dtype')
        if dtype == 'complex128':
            ptr = z_array
        elif dtype == 'complex64':
            ptr = c_array
        elif dtype == 'float64':
            ptr = d_array            
        elif dtype == 'float32':
            ptr = s_array
        s = self.solver
        s.set_rhs(ptr(rhs))            
        if not local:
            s.set_irn(i_array(row))
            s.set_jcn(i_array(col))
            s.set_a(ptr(data))
        else:
            s.set_irn_loc(i_array(row))
            s.set_jcn_loc(i_array(col))
            s.set_a_loc(ptr(data))

def solve(matrix, rhs):
    pass                        





