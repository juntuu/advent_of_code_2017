import Data.Map (empty, findWithDefault, insert)

main = do
  input <- getLine
  print $ part1 $ read input
  print $ part2 $ read input

part1 :: Int -> Int
part1 = uncurry (+) . coord

part2 :: Int -> Int
part2 = fill empty spiral

level = level' [1,3 ..]
  where
    level' (x:xs) n
      | x ^ 2 >= n = x
      | otherwise = level' xs n

coord n = walk (l, l, l, l) (l ^ 2) (l `div` 2, l `div` 2)
  where
    l = level n
    walk moves at (x, y)
      | at == n = (abs x, abs y)
    walk (1, 1, 1, m) at (x, y) = walk (1, 1, 1, m - 1) (at - 1) (x, y + 1)
    walk (1, 1, m, n) at (x, y) = walk (1, 1, m - 1, n) (at - 1) (x + 1, y)
    walk (1, m, n, o) at (x, y) = walk (1, m - 1, n, o) (at - 1) (x, y - 1)
    walk (m, n, o, p) at (x, y) = walk (m - 1, n, o, p) (at - 1) (x - 1, y)

spiral = (0, 0) : walk 1 (0, 0)
  where
    walk lvl at@(x, y) =
      go at (1, 0) (d - 1) ++
      go (lvl, y) (0, 1) (d - 1) ++
      go (lvl, lvl) (-1, 0) d ++
      go (-lvl, lvl) (0, -1) d ++ walk (lvl + 1) (-lvl, -lvl)
      where
        d = 2 * lvl
    go (x, y) (dx, dy) 0 = []
    go (x, y) (dx, dy) steps = step : go step (dx, dy) (steps - 1)
      where
        step = (x + dx, y + dy)

square _ (0, 0) = 1
square m (x, y) = sum $ map (\k -> findWithDefault 0 k m) around
  where
    around =
      [ (x - 1, y + 1)
      , (x, y + 1)
      , (x + 1, y + 1)
      , (x - 1, y)
      , (x + 1, y)
      , (x - 1, y - 1)
      , (x, y - 1)
      , (x + 1, y - 1)
      ]

fill m (x:xs) n
  | n < new = new
  | otherwise = fill (insert x new m) xs n
  where
    new = square m x
