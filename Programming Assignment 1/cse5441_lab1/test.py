def overlap(point_shape, point_exterior):
  if (point_exterior[0] <= point_shape[0] and
    point_exterior[0] + point_exterior[1] <= point_shape[0] + point_shape[1]):
      return point_exterior[1] + point_exterior[0] - point_shape[0]
  elif (point_exterior[0] >= point_shape[0] and
    point_exterior[0] + point_exterior[1] > point_shape[0] + point_shape[1]):
      return point_shape[1] + point_shape[0] - point_exterior[0]
  elif (point_exterior[0] < point_shape[0] and
    point_exterior[0] + point_exterior[1] > point_shape[0] + point_shape[1]):
      return point_shape[1]
  else:
      return point_exterior[1]

def overlap2(box, neighbor):
  nbr_lx = neighbor[0]
  nbr_rx = neighbor[0] + neighbor[1]
  box_lx = box[0]
  box_rx = box[0] + box[1]
  if (nbr_lx <= box_lx and nbr_rx <= box_rx):
    return nbr_rx - box_lx
  elif (nbr_lx >= box_lx and nbr_rx > box_rx):
    return box_rx - nbr_lx
  elif (nbr_lx < box_lx and nbr_rx > box_rx):
    return box_rx - box_lx
  else:
    return nbr_rx - nbr_lx