from dat import *


dat = DATFile('sim_old.dat')
dat.save_csv()
print('Created ' + os.path.splitext(args.filename)[0] + '.csv. ' +
        ('Included' if args.file_refs else 'Excluded') + ' odr and 3D model references.')
dat.close()
