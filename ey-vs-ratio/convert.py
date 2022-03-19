import numpy as np
import pandas as pd

# Load tables
shots = pd.read_excel('../../../database.xlsx', sheet_name='shots', header=0, index_col=0)
configs = pd.read_excel('../../../database.xlsx', sheet_name='trials', header=0, index_col=0)

# Complete split data with full data
shots['split_weight'] = shots['split_weight'].fillna(shots['weight_in_cup'])
shots['split_brix'] = shots['split_brix'].fillna(shots['brix_percent'])

# Trim table
shots = shots[['trial_id', 'weight_in_basket', 'split_weight', 'split_brix', 'notes']]
shots.rename({'trial_id': 'config_id'}, axis=1, inplace=True)
shots.dropna(subset=['config_id'], inplace=True)

# Reset the index
new_id = pd.Series(np.array(range(len(shots))) + 1, name='id')
shots.set_index(new_id, inplace=True)

# Convert data to strings
shots['config_id'] = shots['config_id'].map(lambda x: '%d' % x)
shots['weight_in_basket'] = shots['weight_in_basket'].map(lambda x: '%.2f' % x)
shots['split_weight'] = shots['split_weight'].map(lambda x: '%.2f' % x)
shots['split_brix'] = shots['split_brix'].map(lambda x: '%.1f' % x)

# Write tables
shots.to_csv('shots.csv', float_format='%.2f')
configs.to_csv('configs.csv', float_format='%.2f')
