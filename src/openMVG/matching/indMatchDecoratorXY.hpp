// This file is part of the AliceVision project and is made available under
// the terms of the MPL2 license (see the COPYING.md file).

#ifndef ALICEVISION_MATCHING_IND_MATCH_DECORATOR_XY_H
#define ALICEVISION_MATCHING_IND_MATCH_DECORATOR_XY_H

#include <iostream>
#include "aliceVision/matching/indMatch.hpp"
#include "aliceVision/features/features.hpp"

namespace aliceVision {
namespace matching {

/// IndMatch decorator.
/// Use sorting over x,y coordinates.
template<class T = float>
class IndMatchDecorator
{
  struct IndMatchDecoratorStruct
  {
    IndMatchDecoratorStruct(
      T xa, T ya,
      T xb, T yb,
      const IndMatch & ind) {

      x1 = xa; y1 = ya;
      x2 = xb; y2 = yb;
      index = ind;
    }

    /// Lexicographical ordering of matches. Used to remove duplicates.
    friend bool operator<(const IndMatchDecoratorStruct& m1,
      const IndMatchDecoratorStruct& m2)  {

      if (m1 == m2) return false;

      if (m1.x1 < m2.x1)
        return m1.y1 < m2.y1;
      else
        if (m1.x1 > m2.x1)
          return m1.y1 < m2.y1;
      return m1.x1 < m2.x1;
    }

    /// Comparison Operator
    friend bool operator==(const IndMatchDecoratorStruct& m1,
      const IndMatchDecoratorStruct& m2)  {

      return (m1.x1==m2.x1 && m1.y1==m2.y1 &&
        m1.x2==m2.x2 && m1.y2==m2.y2);
    }

    T x1,y1, x2,y2;
    IndMatch index;
  };
public:

  IndMatchDecorator(const std::vector<IndMatch> & vec_matches,
    const std::vector<features::SIOPointFeature> & leftFeat,
    const std::vector<features::SIOPointFeature> & rightFeat)
    :_vec_matches(vec_matches)
  {
    for (size_t i = 0; i < vec_matches.size(); ++i) {
      const size_t I = vec_matches[i]._i;
      const size_t J = vec_matches[i]._j;
      _vecDecoredMatches.push_back(
        IndMatchDecoratorStruct(leftFeat[I].x(),leftFeat[I].y(),
        rightFeat[J].x(), rightFeat[J].y(), vec_matches[i]));
    }
  }

  IndMatchDecorator(const std::vector<IndMatch> & vec_matches,
    const std::vector<features::PointFeature> & leftFeat,
    const std::vector<features::PointFeature> & rightFeat)
    :_vec_matches(vec_matches)
  {
    for (size_t i = 0; i < vec_matches.size(); ++i) {
      const size_t I = vec_matches[i]._i;
      const size_t J = vec_matches[i]._j;
      _vecDecoredMatches.push_back(
        IndMatchDecoratorStruct(leftFeat[I].x(),leftFeat[I].y(),
        rightFeat[J].x(), rightFeat[J].y(), vec_matches[i]));
    }
  }

  IndMatchDecorator(const std::vector<IndMatch> & vec_matches,
    const Mat & leftFeat,
    const Mat & rightFeat)
    :_vec_matches(vec_matches)
  {
    for (size_t i = 0; i < vec_matches.size(); ++i) {
      const size_t I = vec_matches[i]._i;
      const size_t J = vec_matches[i]._j;
      _vecDecoredMatches.push_back(
        IndMatchDecoratorStruct(leftFeat.col(I)(0),leftFeat.col(I)(1),
        rightFeat.col(J)(0), rightFeat.col(J)(1), vec_matches[i]));
    }
  }

  /// Remove duplicates (same (x1,y1) coords that appears multiple times)
  size_t getDeduplicated(std::vector<IndMatch> & vec_matches)
  {
    size_t sizeBefore = _vecDecoredMatches.size();
    std::set<IndMatchDecoratorStruct> set_deduplicated(
      _vecDecoredMatches.begin(),_vecDecoredMatches.end());
    _vecDecoredMatches.assign(set_deduplicated.begin(), set_deduplicated.end());

    vec_matches.resize(_vecDecoredMatches.size());
    for (size_t i = 0; i < _vecDecoredMatches.size(); ++i)  {
      const IndMatch & idxM = _vecDecoredMatches[i].index;
      vec_matches[i] = idxM;
    }

    return sizeBefore != vec_matches.size();
  }

  /**
    * Save the corresponding matches to file.
    * \param nameFile   The file where matches will be saved.
    * \param vec_match  The matches that we want to save.
    * \return bool True if everything was ok, otherwise false.
    */
    bool saveMatch(const char* nameFile) const  {
      std::ofstream f(nameFile);
      if( f.is_open() ) {
        std::copy(_vecDecoredMatches.begin(), _vecDecoredMatches.end(),
          std::ostream_iterator<IndMatchDecoratorStruct>(f, ""));
      }
      return f.is_open();
    }

    friend std::ostream& operator<<(std::ostream& os, const IndMatchDecoratorStruct & m)
    {
      return os << m.x1 << " " << m.y1 << " " << m.x2 << " " << m.y2 << "\n";
    }

private :
  std::vector<IndMatch> _vec_matches;
  std::vector<IndMatchDecoratorStruct> _vecDecoredMatches;
};

}  // namespace matching
}  // namespace aliceVision

#endif // ALICEVISION_MATCHING_IND_MATCH_DECORATOR_XY_H
