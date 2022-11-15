-- Copyright (C) 2022  Xiaoyue Chen
--
-- This program is free software: you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation, either version 3 of the License, or
-- (at your option) any later version.
--
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with this program.  If not, see <http://www.gnu.org/licenses/>.

import Control.Monad.State
import Data.Bits
import Data.Word
import System.Environment
import Text.Printf

data Address = Address Int deriving (Eq)

data Tag = Tag Bool Address

data Data = Data [Word32]

data Block = Block Tag Data

data Set = Set [Block]

data Cache = Cache Int Int [Set]

data CacheState = CacheState Int Int

instance Show Address where
  show (Address addr) = printf "0x%x" addr

instance Show Tag where
  show (Tag valid address) =
    printf
      "%c:%s"
      (if valid then 'V' else '-')
      (show address)

instance Show Data where
  show (Data _) = ""

instance Show Block where
  show (Block tag dat) = printf "%s %s" (show tag) (show dat)

instance Show Set where
  show (Set []) = ""
  show (Set (blk : blks)) =
    printf
      "%s%s"
      (show blk)
      (if null blks then "" else " ")
      ++ show (Set blks)

instance Show Cache where
  show (Cache _ _ sets) =
    concatMap (printf "%s\n" . show) sets

instance Show CacheState where
  show (CacheState naccess nhit) =
    printf
      "Access: %d  Hit: %d  Miss: %d  Miss Rate: %f%%"
      naccess
      nhit
      (naccess - nhit)
      ( (fromIntegral (naccess - nhit) :: Float)
          / (fromIntegral naccess :: Float) * 100
      )

wordSize :: Int
wordSize = 4

blkAddr :: Address -> Int -> Address
blkAddr (Address addr) blksize = Address $ addr .&. complement (blksize - 1)

setIdx :: Address -> Int -> Int -> Int
setIdx (Address addr) nset blksize =
  (addr .&. (nset * blksize * 8 - 1))
    `shiftR` countTrailingZeros (blksize * 8)

create :: Int -> Int -> Int -> Cache
create nset assoc blksize =
  Cache
    assoc
    blksize
    [ Set
        [ Block
            (Tag False $ Address 0)
            (Data [0 | _ <- [1 .. blksize `div` wordSize]])
          | _ <- [1 .. assoc]
        ]
      | _ <- [1 .. nset]
    ]

access :: Cache -> Address -> State CacheState Cache
access (Cache assoc blksize sets) addr = do
  (CacheState naccess nhit) <- get
  put $ CacheState (naccess + 1) (if hit then nhit + 1 else nhit)
  return $ Cache assoc blksize $ before ++ (set' : after)
  where
    nset = length sets
    blkaddr = blkAddr addr blksize
    idx = setIdx blkaddr nset blksize
    (before, set : after) = splitAt idx sets
    (set', hit) = accessSet set blkaddr

updateBlk :: Block -> Address -> Block
updateBlk (Block (Tag _ _) (Data dat)) addr = Block (Tag True addr) (Data dat)

ownBlk :: Block -> Address -> Bool
ownBlk (Block (Tag True tagaddr) _) addr = tagaddr == addr
ownBlk _ _ = False

accessSet :: Set -> Address -> (Set, Bool)
accessSet (Set blks) addr = (Set blks', hit)
  where
    (blks', hit) = accessSet' [] blks addr

accessSet' :: [Block] -> [Block] -> Address -> ([Block], Bool)
accessSet' looked [lst] addr =
  if ownBlk lst addr
    then (lst : looked, True)
    else (updateBlk lst addr : looked, False)
accessSet' looked (blk : blks) addr =
  if ownBlk blk addr
    then (blk : looked ++ blks, True)
    else accessSet' (looked ++ [blk]) blks addr

main :: IO ()
main = do
  [nset, assoc, blksize, file] <- getArgs
  addrs <- map (Address . read) . lines <$> readFile file
  let cache = create (read nset) (read assoc) (read blksize)
      state = CacheState 0 0
      (cache', state') =
        foldl
          (\(acc, stat) addr -> runState (access acc addr) stat)
          (cache, state)
          addrs
  print cache'
  print state'
